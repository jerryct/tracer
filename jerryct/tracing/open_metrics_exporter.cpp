// SPDX-License-Identifier: MIT

#include "jerryct/tracing/open_metrics_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <errno.h>
#include <exception>
#include <fmt/core.h>
#include <fmt/format.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
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

HttpServer::HttpServer()
    : listener_{::socket(AF_INET, SOCK_STREAM, 0)}, fds{}, request_{}, response_{},
      poll_failed_{Meter(), "http_poll_failed"}, revents_{Meter(), "http_revents_not_pollin"},
      accept_failed_{Meter(), "http_accept_failed"}, read_failed_{Meter(), "http_read_failed"},
      write_failed_{Meter(), "http_write_failed"}, connection_opened_{Meter(), "http_connection_opened"},
      connection_closed_{Meter(), "http_connection_closed"}, client_closed_{Meter(),
                                                                            "http_connection_closed_by_client"},
      bytes_written_{Meter(), "http_bytes_written"}, server_resets_{Meter(), "http_server_resets"} {
  request_.resize(32000U);
  response_.reserve(1024U);

  if (!listener_.IsValid()) {
    throw std::runtime_error{"cannot create socket"};
  }

  const int reuse{1};
  if (::setsockopt(listener_.fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    throw std::runtime_error{"cannot set socket to reuse address"};
  }

  const int non_blocking{1};
  if (::ioctl(listener_.fd_, FIONBIO, &non_blocking)) {
    throw std::runtime_error{"cannot set socket to non blocking"};
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(9110);

  if (::bind(listener_.fd_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
    throw std::runtime_error{"cannot bind address"};
  }
  if (::listen(listener_.fd_, 32) < 0) {
    throw std::runtime_error{"cannot listen for connections"};
  }

  fds.push_back(pollfd{listener_.fd_, POLLIN, 0});
}

bool HttpServer::IsAlive() const { return !fds.empty(); }

// https://www.ibm.com/docs/en/i/7.1?topic=designs-using-poll-instead-select
void HttpServer::Step(fmt::memory_buffer &content_) {
  bool end_server{false};

  do {
    bool compress_array{false};

    const ssize_t rc0{::poll(fds.data(), fds.size(), 1)};
    if (rc0 < 0) {
      poll_failed_.Add();
      end_server = true;
      break;
    }
    if (rc0 == 0) {
      break;
    }

    const std::size_t current_size{fds.size()};
    for (std::size_t i{0U}; i < current_size; ++i) {
      if (fds[i].revents == 0) {
        continue;
      }

      if (fds[i].revents != POLLIN) { //  If revents is not POLLIN, it's an unexpected result, end the server.
        fmt::print("Error! revents = {}\n", fds[i].revents);
        revents_.Add();
        end_server = true;
        break;
      }

      if (fds[i].fd == listener_.fd_) {
        int new_sd;
        do {
          new_sd = ::accept(listener_.fd_, nullptr, nullptr);
          if (new_sd < 0) {
            if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
              accept_failed_.Add();
              end_server = true;
            }
            break;
          }

          connection_opened_.Add();
          fds.push_back(pollfd{new_sd, POLLIN, 0});
        } while (new_sd != -1);
      } else {
        bool close_conn{false};

        do {
          const ssize_t rc1{::read(fds[i].fd, request_.data(), request_.size())};
          if (rc1 < 0) {
            if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
              read_failed_.Add();
              close_conn = true;
            }
            break;
          }
          if (rc1 == 0) {
            client_closed_.Add();
            close_conn = true;
            break;
          }

          response_.append(
              fmt::string_view{"HTTP/1.1 200 OK\r\nContent-Type: text/plain; version=0.0.4\r\nContent-Length: "});
          response_.append(fmt::format_int{content_.size()});
          response_.append(fmt::string_view{"\r\n\r\n"});

          std::array<::iovec, 2> v;
          v[0].iov_base = &response_[0];
          v[0].iov_len = response_.size();
          v[1].iov_base = &content_[0];
          v[1].iov_len = content_.size();

          const ssize_t rc2{::writev(fds[i].fd, v.data(), v.size())};
          if (rc2 < 0) {
            write_failed_.Add();
            close_conn = true;
            break;
          }
          bytes_written_.Add(rc2);
          response_.clear();
        } while (false);

        if (close_conn) {
          connection_closed_.Add();
          ::close(fds[i].fd);
          fds[i].fd = -1;
          compress_array = true;
        }
      }
    }

    if (compress_array) {
      compress_array = false;
      const auto it = std::remove_if(fds.begin(), fds.end(), [](const pollfd &fd) { return fd.fd == -1; });
      fds.erase(it, fds.end());
    }
  } while (false);

  if (end_server) {
    server_resets_.Add();
    for (std::size_t i{0U}; i < fds.size(); ++i) {
      if (fds[i].fd == listener_.fd_) {
        listener_ = {};
      } else if (fds[i].fd >= 0) {
        connection_closed_.Add();
        ::close(fds[i].fd);
      } else {
      }
    }
    fds.clear();
  }
}

void OpenMetricsExporter::operator()(const std::unordered_map<string_view, std::uint64_t> &counters) {
  content_.reserve(1024U);
  content_.clear();

  for (const auto &c : counters) {
    content_.append(fmt::string_view{"# TYPE "});
    content_.append(c.first);
    content_.append(fmt::string_view{"_total counter\n"});
    content_.append(c.first);
    content_.append(fmt::string_view{"_total "});
    content_.append(fmt::format_int{c.second});
    content_.push_back('\n');
  }
}

void OpenMetricsExporter::Expose() {
  if (!server_.IsAlive()) {
    server_ = {};
  }
  server_.Step(content_);
}

} // namespace trace
} // namespace jerryct
