// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_CHROME_TRACING_H
#define JERRYCT_TRACING_CHROME_TRACING_H

#include "jerryct/tracing/tracing.h"
#include "nlohmann/json.hpp"
#include <chrono>
#include <fstream>
#include <string>

namespace jerryct {
namespace trace {

struct AsChromeTracingJson {
  void operator()(const int tid, const jerryct::string_view name, const Phase p,
                  const std::chrono::steady_clock::time_point ts) {
    const auto d = std::chrono::duration<double, std::micro>{ts.time_since_epoch()}.count();

    if (p == Phase::begin)
      j.push_back({{"name", std::string{name.data(), name.size()}}, {"pid", 0}, {"tid", tid}, {"ph", "B"}, {"ts", d}});
    if (p == Phase::end)
      j.push_back({{"pid", 0}, {"tid", tid}, {"ph", "E"}, {"ts", d}});
  }

  AsChromeTracingJson(std::string filename) : j{nlohmann::json::array()}, filename_{filename} {}
  ~AsChromeTracingJson() {
    std::ofstream o(filename_);
    o << j << std::endl;
  }

  nlohmann::json j;
  std::string filename_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_CHROME_TRACING_H
