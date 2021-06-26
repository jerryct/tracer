// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H
#define JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H

#include "jerryct/tracing/tracing.h"
#include <chrono>
#include <cstdio>
#include <string>

namespace jerryct {
namespace trace {

class ChromeTraceEventExporter {
public:
  ChromeTraceEventExporter(const std::string &filename) : f_{fopen(filename.c_str(), "w")} {
    if (!IsValid()) {
      throw;
    }
    fprintf(f_, "[{}");
  }
  ChromeTraceEventExporter(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter(ChromeTraceEventExporter &&other) noexcept : f_{other.f_} { other.f_ = nullptr; }
  ChromeTraceEventExporter &operator=(const ChromeTraceEventExporter &) = delete;
  ChromeTraceEventExporter &operator=(ChromeTraceEventExporter &&other) noexcept {
    if (f_ != other.f_) {
      std::swap(f_, other.f_);
    }
    return *this;
  }
  ~ChromeTraceEventExporter() noexcept {
    if (IsValid()) {
      fprintf(f_, "]");
      fclose(f_);
    }
  }
  bool IsValid() const { return f_ != nullptr; }

  void operator()(const int tid, const std::uint64_t losts, const std::vector<Event> &events) {
    for (const Event &e : events) {
      const auto d = std::chrono::duration<double, std::micro>{e.ts.time_since_epoch()}.count();

      if (e.p == Phase::begin) {
        const jerryct::string_view v{e.name.get()};
        fprintf(f_, R"(,{"name":"%.*s","pid":0,"tid":%d,"ph":"B","ts":%f})", static_cast<int>(v.size()), v.data(), tid,
                d);
      }
      if (e.p == Phase::end) {
        fprintf(f_, R"(,{"pid":0,"tid":%d,"ph":"E","ts":%f})", tid, d);
      }
    }
    const auto d = std::chrono::duration<double, std::micro>{std::chrono::steady_clock::now().time_since_epoch()};
    fprintf(f_, R"(,{"pid":0,"name":"total lost events","ph":"C","ts":%f,"args":{"value":%lu}})", d.count(), losts);
  }

private:
  std::FILE *f_{nullptr};
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_CHROME_TRACE_EVENT_EXPORTER_H
