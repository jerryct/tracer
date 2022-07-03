// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_OPEN_METRICS_EXPORTER_H
#define JERRYCT_TELEMETRY_OPEN_METRICS_EXPORTER_H

#include "jerryct/string_view.h"
#include "jerryct/tracing/tracing.h"
#include <cstdint>
#include <fmt/format.h>
#include <poll.h>
#include <unordered_map>
#include <vector>

namespace jerryct {
namespace telemetry {

struct FileDesc {
  FileDesc() noexcept = default;
  FileDesc(int fd);
  FileDesc(const FileDesc &) = delete;
  FileDesc(FileDesc &&other) noexcept;
  FileDesc &operator=(const FileDesc &) = delete;
  FileDesc &operator=(FileDesc &&other) noexcept;
  ~FileDesc() noexcept;
  bool IsValid() const;

  int fd_{-1};
};

class HttpServer {
public:
  HttpServer();

  bool IsAlive() const;
  void Step(fmt::memory_buffer &content_);

private:
  FileDesc listener_;
  std::vector<::pollfd> fds;

  std::vector<char> request_;
  fmt::memory_buffer response_;

  Counter poll_failed_;
  Counter revents_;
  Counter accept_failed_;
  Counter read_failed_;
  Counter write_failed_;
  Counter connection_opened_;
  Counter connection_closed_;
  Counter client_closed_;
  Counter bytes_written_;
  Counter server_resets_;
};

class OpenMetricsExporter {
public:
  void operator()(const std::unordered_map<string_view, std::uint64_t> &counters);
  void Expose();

private:
  fmt::memory_buffer content_;

  HttpServer server_{};
};

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_OPEN_METRICS_EXPORTER_H
