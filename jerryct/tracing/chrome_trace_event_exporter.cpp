// SPDX-License-Identifier: MIT

#include "jerryct/tracing/chrome_trace_event_exporter.h"
#include <chrono>
#include <cstdio>
#include <string>

namespace jerryct {
namespace trace {

File::File(const std::string &filename) : f_{::fopen(filename.c_str(), "w")} {
  if (!IsValid()) {
    throw;
  }
}

File::File(File &&other) noexcept : f_{other.f_} { other.f_ = nullptr; }

File &File::operator=(File &&other) noexcept {
  if (f_ != other.f_) {
    std::swap(f_, other.f_);
  }
  return *this;
}

File::~File() noexcept {
  if (IsValid()) {
    ::fclose(f_);
  }
}
bool File::IsValid() const { return f_ != nullptr; }

ChromeTraceEventExporter::ChromeTraceEventExporter(const std::string &filename) : f_{filename} {
  fprintf(f_.Get(), "[{}");
}

ChromeTraceEventExporter::~ChromeTraceEventExporter() noexcept { fprintf(f_.Get(), "]"); }

void ChromeTraceEventExporter::operator()(const int tid, const std::int64_t losts, const std::vector<Event> &events) {
  for (const Event &e : events) {
    const auto d = std::chrono::duration<double, std::micro>{e.ts.time_since_epoch()}.count();

    if (e.p == Phase::begin) {
      const jerryct::string_view v{e.name.Get()};
      fprintf(f_.Get(), R"(,{"name":"%.*s","pid":0,"tid":%d,"ph":"B","ts":%f})", static_cast<int>(v.size()), v.data(),
              tid, d);
    }
    if (e.p == Phase::end) {
      fprintf(f_.Get(), R"(,{"pid":0,"tid":%d,"ph":"E","ts":%f})", tid, d);
    }
  }
  const auto d = std::chrono::duration<double, std::micro>{std::chrono::steady_clock::now().time_since_epoch()};
  fprintf(f_.Get(), R"(,{"pid":0,"name":"total lost events","ph":"C","ts":%f,"args":{"value":%ld}})", d.count(), losts);
}

} // namespace trace
} // namespace jerryct
