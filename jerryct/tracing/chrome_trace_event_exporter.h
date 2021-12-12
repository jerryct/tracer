// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H
#define JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H

#include "jerryct/tracing/tracing.h"
#include <chrono>
#include <cstdio>
#include <fmt/format.h>
#include <fmt/os.h>
#include <string>

namespace jerryct {
namespace trace {

class FileRotate {
public:
  explicit FileRotate(const std::string &filename);
  void Rotate();
  std::FILE *Get() const;

private:
  fmt::buffered_file f_;
  std::int32_t rotation_size = 5;
  std::string filename_;
};

class ChromeTraceEventExporter {
public:
  explicit ChromeTraceEventExporter(const std::string &filename);
  ChromeTraceEventExporter(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter(ChromeTraceEventExporter &&other) = default;
  ChromeTraceEventExporter &operator=(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter &operator=(ChromeTraceEventExporter &&other) = default;
  ~ChromeTraceEventExporter() noexcept;

  void operator()(const std::int32_t tid, const std::uint64_t losts, const std::vector<Event> &events);

  void Rotate();

private:
  FileRotate f_;
  fmt::memory_buffer buf_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H
