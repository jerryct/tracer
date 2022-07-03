// SPDX-License-Identifier: MIT

#include "jerryct/tracing/tracing.h"

namespace jerryct {
namespace telemetry {

Span::Span(TracerImpl &t, const jerryct::string_view name) : t_{t.PerThreadEvents()} {
  const auto now = std::chrono::steady_clock::now();
  t_->events.Emplace(Phase::begin, now, name);
}

Span::~Span() noexcept {
  const auto now = std::chrono::steady_clock::now();
  t_->events.Emplace(Phase::end, now, jerryct::string_view{""});
}

Counter::Counter(MeterImpl &t, const jerryct::string_view name) : t_{&t}, id_{t_->RegisterName(name)} {
  t_->PerThreadEvents()->events.Emplace(0U, id_);
}

void Counter::Add() { t_->PerThreadEvents()->events.Emplace(1U, id_); }
void Counter::Add(const std::int64_t v) { t_->PerThreadEvents()->events.Emplace(static_cast<std::uint64_t>(v), id_); }

} // namespace telemetry
} // namespace jerryct
