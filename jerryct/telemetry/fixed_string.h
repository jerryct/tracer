// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_FIXED_STRING_H
#define JERRYCT_TELEMETRY_FIXED_STRING_H

#include "jerryct/string_view.h"
#include <algorithm>
#include <cstring>

namespace jerryct {
namespace telemetry {

class FixedString {
public:
  FixedString() noexcept : s_{0} {}

  FixedString(const jerryct::string_view v) noexcept {
    if (v.empty()) {
      s_ = 0;
      return;
    }
    s_ = std::min(64UL, v.size());
    std::memmove(&d_[0], v.data(), s_);
  }

  jerryct::string_view Get() const { return {&d_[0], s_}; }

private:
  char d_[64];
  std::size_t s_;
};

inline bool operator==(const FixedString &lhs, const FixedString &rhs) noexcept { return lhs.Get() == rhs.Get(); }
inline bool operator<(const FixedString &lhs, const FixedString &rhs) noexcept { return lhs.Get() < rhs.Get(); }

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_FIXED_STRING_H
