// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H
#define JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H

#include "jerryct/tracing/tracing.h"
#include <chrono>
#include <cstdio>
#include <string>

namespace jerryct {
namespace trace {

struct File {
  File() noexcept = default;
  File(const std::string &filename);
  File(const File &) = delete;
  File(File &&other) noexcept;
  File &operator=(const File &) = delete;
  File &operator=(File &&other) noexcept;
  ~File() noexcept;
  bool IsValid() const;

  std::FILE *f_{nullptr};
};

class FileRotate {
public:
  explicit FileRotate(const std::string &filename);
  void Rotate();
  std::FILE *Get() const;

private:
  File f_;
  int rotation_size = 5;
  std::string filename_;
};

class ChromeTraceEventExporter {
public:
  explicit ChromeTraceEventExporter(const std::string &filename);
  ChromeTraceEventExporter(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter(ChromeTraceEventExporter &&other) noexcept = default;
  ChromeTraceEventExporter &operator=(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter &operator=(ChromeTraceEventExporter &&other) noexcept = default;
  ~ChromeTraceEventExporter() noexcept;

  void operator()(const std::int32_t tid, const std::uint64_t losts, const std::vector<Event> &events);

  void Rotate();

private:
  FileRotate f_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H
