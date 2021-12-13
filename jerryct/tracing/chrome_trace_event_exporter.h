// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H
#define JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H

#include "jerryct/tracing/tracing.h"
#include <cstdint>
#include <cstdio>
#include <fmt/format.h>
#include <fmt/os.h>
#include <string>
#include <vector>

namespace jerryct {
namespace trace {

class FileRotate {
public:
  explicit FileRotate(const std::string &filename);
  void Rotate();
  std::FILE *Get() const;

private:
  fmt::buffered_file f_;
  std::int32_t rotation_size_;
  std::string filename_;
};

class ChromeTraceEventExporter {
public:
  explicit ChromeTraceEventExporter(const std::string &filename);
  ChromeTraceEventExporter(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter &operator=(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter(ChromeTraceEventExporter &&other) = default;
  ChromeTraceEventExporter &operator=(ChromeTraceEventExporter &&other);
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
