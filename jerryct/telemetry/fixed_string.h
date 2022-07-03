// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_FIXED_STRING_H
#define JERRYCT_TELEMETRY_FIXED_STRING_H

#include "jerryct/string_view.h"
#include <algorithm>
#include <cstring>

namespace jerryct {
namespace telemetry {

template <std::size_t N> class FixedString {
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

  static std::size_t Size() { return N; }

private:
  char d_[N];
  std::size_t s_;
};

template <std::size_t N1, std::size_t N2>
bool operator==(const FixedString<N1> &lhs, const FixedString<N2> &rhs) noexcept {
  return lhs.Get() == rhs.Get();
}
template <std::size_t N1, std::size_t N2>
bool operator<(const FixedString<N1> &lhs, const FixedString<N2> &rhs) noexcept {
  return lhs.Get() < rhs.Get();
}

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_FIXED_STRING_H
