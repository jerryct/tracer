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
  FileRotate(const std::string &filename) : f_{}, filename_{filename} { Rotate(); }

  void Rotate() {
    ::remove((filename_ + std::to_string(rotation_size - 1)).c_str());
    for (int i{rotation_size - 1}; i > 1; --i) {
      ::rename((filename_ + std::to_string(i - 1)).c_str(), (filename_ + std::to_string(i)).c_str());
    }
    ::rename(filename_.c_str(), (filename_ + std::to_string(1)).c_str());

    f_ = {filename_};
  }

  std::FILE *Get() const { return f_.f_; }

private:
  File f_;
  int rotation_size = 5;
  std::string filename_;
};

class ChromeTraceEventExporter {
public:
  ChromeTraceEventExporter(const std::string &filename);
  ChromeTraceEventExporter(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter(ChromeTraceEventExporter &&other) noexcept = default;
  ChromeTraceEventExporter &operator=(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter &operator=(ChromeTraceEventExporter &&other) noexcept = default;
  ~ChromeTraceEventExporter() noexcept;

  void operator()(const int tid, const std::int64_t losts, const std::vector<Event> &events);

  void Rotate() {
    fprintf(f_.Get(), "]");
    f_.Rotate();
    fprintf(f_.Get(), "[{}");
  }

private:
  FileRotate f_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H
