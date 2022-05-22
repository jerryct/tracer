// SPDX-License-Identifier: MIT

#include "jerryct/tracing/tracing.h"

namespace jerryct {
namespace trace {

Span::Span(TracerImpl &t, const jerryct::string_view name) : t_{t.PerThreadEvents()} {
  const auto now = std::chrono::steady_clock::now();
  t_->events.Emplace(Phase::begin, now, name);
}

Span::~Span() noexcept {
  const auto now = std::chrono::steady_clock::now();
  t_->events.Emplace(Phase::end, now, jerryct::string_view{""});
}

void Meter::Increment() { t_->PerThreadEvents()->counters[name_] += 1; }

} // namespace trace
} // namespace jerryct
