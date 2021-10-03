// SPDX-License-Identifier: MIT

#include "jerryct/tracing/prometheus_exporter.h"
#include <arpa/inet.h>
#include <array>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace jerryct {
namespace trace {

FileDesc::FileDesc(int fd) : fd_{fd} {}

FileDesc::FileDesc(FileDesc &&other) noexcept : fd_{other.fd_} { other.fd_ = -1; }

FileDesc &FileDesc::operator=(FileDesc &&other) noexcept {
  if (fd_ != other.fd_) {
    std::swap(fd_, other.fd_);
  }
  return *this;
}

FileDesc::~FileDesc() noexcept {
  if (IsValid()) {
    ::close(fd_);
  }
}

bool FileDesc::IsValid() const { return fd_ != -1; }

JoinThread::JoinThread(JoinThread &&other) noexcept : stop_token_{-1}, t_{} {
  std::swap(stop_token_, other.stop_token_);
  std::swap(t_, other.t_);
}

JoinThread &JoinThread::operator=(JoinThread &&other) noexcept {
  if (t_.joinable()) {
    ::shutdown(stop_token_, SHUT_RDWR);
    t_.join();
  }
  std::swap(stop_token_, other.stop_token_);
  std::swap(t_, other.t_);

  return *this;
}

JoinThread::~JoinThread() {
  if (t_.joinable()) {
    ::shutdown(stop_token_, SHUT_RDWR);
    t_.join();
  }
}

RequestHandler::RequestHandler(FileDesc fd) : fd_{std::move(fd)} {
  request_.resize(32000);
  response_.reserve(1024);
  content_.reserve(1024);
}

void RequestHandler::operator()(PrometheusExporter &exporter) {
  while (true) {
    if (::read(fd_.fd_, request_.data(), request_.size()) <= 0) {
      break;
    }

    exporter.Expose(content_);

    response_.append("HTTP/1.1 200 OK\r\nContent-Type: text/plain; version=0.0.4\r\nContent-Length: ");
    response_.append(std::to_string(content_.size()));
    response_.append("\r\n\r\n");

    std::array<::iovec, 2> v;
    v[0].iov_base = &response_[0];
    v[0].iov_len = response_.size();
    v[1].iov_base = &content_[0];
    v[1].iov_len = content_.size();
    if (::writev(fd_.fd_, v.data(), v.size()) < 0) {
      break;
    }

    response_.clear();
    content_.clear();
  }
}

ConnectionHandler::ConnectionHandler(PrometheusExporter &exporter)
    : fd_{::socket(AF_INET, SOCK_STREAM, 0)}, connections_{}, awaiter_{} {
  if (!fd_.IsValid()) {
    throw;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(9110);

  const int reuse{1};
  if (::setsockopt(fd_.fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    throw;
  }

  if (::bind(fd_.fd_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
    throw;
  }
  if (::listen(fd_.fd_, 2) < 0) {
    throw;
  }

  awaiter_ = JoinThread{fd_.fd_, &ConnectionHandler::Await, this, std::ref(exporter)};
}

void ConnectionHandler::Await(PrometheusExporter &exporter) {
  while (true) {
    const int new_socket{::accept(fd_.fd_, nullptr, nullptr)};
    if (new_socket < 0) {
      break;
    }

    connections_.remove_if([](const Async &a) { return a.HasFinished(); });
    connections_.emplace_front(new_socket, RequestHandler{new_socket}, std::ref(exporter));
  }
}

void PrometheusExporter::operator()(const std::int32_t tid, const std::uint64_t losts,
                                    const std::vector<Event> &events) {
  std::lock_guard<std::mutex> lk{m_};
  auto &stack = stacks_[tid];
  for (const Event &e : events) {
    switch (e.phase) {
    case Phase::begin:
      stack.push_back({{e.name.Get().data(), e.name.Get().size()}, e.time_stamp});
      break;
    case Phase::end:
      if (!stack.empty()) {
        auto &data = data_[stack.back().name];
        const auto d = e.time_stamp - stack.back().ts;
        data.min = d < data.min ? d : data.min;
        data.max = d > data.max ? d : data.max;
        data.sum += d;
        ++data.count;
        stack.pop_back();
      }
      break;
    }
  }
  losts_[tid] = losts;
}

namespace {

void to_chars(const std::int64_t v, std::string &s) {
  static_assert(20 == std::numeric_limits<std::uint64_t>::digits10 + 1, "");
  constexpr int buffer_length = 21; // 20 digits + sign

  uint64_t value = static_cast<std::uint64_t>(v);
  if (v < 0) {
    value = ~value + 1U;
  }
  char temp[buffer_length];
  char *p = &temp[buffer_length];
  do {
    --p;
    *p = static_cast<char>(static_cast<char>(value % 10U) + '0');
    value /= 10U;
  } while (value > 0);
  if (v < 0) {
    *--p = '-';
  }

  s.append(p, static_cast<std::size_t>(&temp[buffer_length] - p));
}

} // namespace

void PrometheusExporter::Expose(std::string &content) {
  std::lock_guard<std::mutex> lk{m_};
  content.append("# TYPE duration_nanoseconds summary\n");
  for (auto &v : data_) {
    content.append("duration_nanoseconds{name=\"");
    content.append(v.first);
    content.append("\",quantile=\"0\"} ");
    to_chars(v.second.min.count(), content);
    content.append("\nduration_nanoseconds{name=\"");
    content.append(v.first);
    content.append("\",quantile=\"1\"} ");
    to_chars(v.second.max.count(), content);
    content.append("\nduration_nanoseconds_sum{name=\"");
    content.append(v.first);
    content.append("\"} ");
    to_chars(v.second.sum.count(), content);
    content.append("\nduration_nanoseconds_count{name=\"");
    content.append(v.first);
    content.append("\"} ");
    to_chars(v.second.count, content);
    content.append("\n");

    v.second.min = std::chrono::nanoseconds::max();
    v.second.max = {};
  }

  content.append("# TYPE lost_events_total counter\n");
  for (const auto &v : losts_) {
    content.append("lost_events_total{tid=\"");
    to_chars(v.first, content);
    content.append("\"} ");
    to_chars(v.second, content);
    content.append("\n");
  }
}

} // namespace trace
} // namespace jerryct
