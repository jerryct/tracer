// SPDX-License-Identifier: MIT

#include "jerryct/tracing/prometheus_exporter.h"
#include <arpa/inet.h>
#include <array>
#include <fmt/core.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace jerryct {
namespace trace {

FileDesc::FileDesc(int fd) : fd_{fd} {}

FileDesc::FileDesc(FileDesc &&other) noexcept : fd_{other.fd_} { other.fd_ = -1; }

FileDesc &FileDesc::operator=(FileDesc &&other) noexcept {
  if (IsValid()) {
    ::close(fd_);
  }
  fd_ = other.fd_;
  other.fd_ = -1;
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

    response_.append(
        fmt::string_view{"HTTP/1.1 200 OK\r\nContent-Type: text/plain; version=0.0.4\r\nContent-Length: "});
    response_.append(fmt::format_int{content_.size()});
    response_.append(fmt::string_view{"\r\n\r\n"});

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

void PrometheusExporter::operator()(const std::unordered_map<string_view, std::int64_t> &counters) {
  std::lock_guard<std::mutex> lk{m_};
  counters_ = counters;
}

void PrometheusExporter::Expose(fmt::memory_buffer &content) {
  std::lock_guard<std::mutex> lk{m_};
  for (const auto &v : counters_) {
    content.append(fmt::string_view{"# TYPE "});
    content.append(v.first);
    content.append(fmt::string_view{"_total counter\n"});
    content.append(v.first);
    content.append(fmt::string_view{"_total "});
    content.append(fmt::format_int{v.second});
    content.push_back('\n');
  }
}

} // namespace trace
} // namespace jerryct
