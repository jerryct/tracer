// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/span.h"
#include <chrono>

namespace jerryct {
namespace telemetry {

Span::Span(TracerImpl &t, const jerryct::string_view name) : t_{t.PerThreadEvents()} {
  const auto now = std::chrono::steady_clock::now();
  t_->Emplace(Phase::begin, now, name);
}

Span::~Span() noexcept {
  const auto now = std::chrono::steady_clock::now();
  t_->Emplace(Phase::end, now, jerryct::string_view{""});
}

} // namespace telemetry
} // namespace jerryct
