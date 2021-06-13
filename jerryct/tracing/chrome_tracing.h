// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_CHROME_TRACING_H
#define JERRYCT_TRACING_CHROME_TRACING_H

#include "jerryct/tracing/tracing.h"
#include <chrono>
#include <cstdio>
#include <string>

namespace jerryct {
namespace trace {

class AsChromeTracingJson {
public:
  AsChromeTracingJson(const std::string &filename) : f_{fopen(filename.c_str(), "w")} {
    if (!IsValid()) {
      throw;
    }
    fprintf(f_, "[{}");
  }
  AsChromeTracingJson(const AsChromeTracingJson &) = delete;
  AsChromeTracingJson(AsChromeTracingJson &&other) noexcept : f_{other.f_} { other.f_ = nullptr; }
  AsChromeTracingJson &operator=(const AsChromeTracingJson &) = delete;
  AsChromeTracingJson &operator=(AsChromeTracingJson &&other) noexcept {
    if (f_ != other.f_) {
      std::swap(f_, other.f_);
    }
    return *this;
  }
  ~AsChromeTracingJson() noexcept {
    if (IsValid()) {
      fprintf(f_, "]");
      fclose(f_);
    }
  }
  bool IsValid() const { return f_ != nullptr; }

  void operator()(const int tid, const std::vector<Event> &events) {
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
  }

private:
  std::FILE *f_{nullptr};
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_CHROME_TRACING_H
