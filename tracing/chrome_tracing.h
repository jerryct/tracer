// SPDX-License-Identifier: MIT

#ifndef TRACING_CHROME_TRACING_H
#define TRACING_CHROME_TRACING_H

#include "json.hpp"
#include "tracing.h"
#include <chrono>
#include <fstream>
#include <string>

namespace trace {

struct AsChromeTracingJson {
  void operator()(const int tid, const Attributes &a, const Phase p, const std::chrono::steady_clock::time_point ts) {
    const auto d = std::chrono::duration<double, std::micro>{ts.time_since_epoch()}.count();

    switch (a.type) {
    case Type::async: {
      auto phase = p == Phase::begin ? "b" : "e";
      j.push_back({{"name", a.name}, {"cat", a.tag}, {"pid", 0}, {"tid", tid}, {"id", tid}, {"ph", phase}, {"ts", d}});
      break;
    }
    case Type::duration: {
      auto phase = p == Phase::begin ? "B" : "E";
      j.push_back({{"name", a.name}, {"cat", a.tag}, {"pid", 0}, {"tid", tid}, {"ph", phase}, {"ts", d}});
      break;
    }
    }
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

#endif // TRACING_CHROME_TRACING_H
