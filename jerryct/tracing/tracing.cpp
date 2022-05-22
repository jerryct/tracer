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

Meter::Meter(MetricsImpl &t, const jerryct::string_view name) : t_{&t}, id_{t_->id_.fetch_add(1)} {
  if (id_ >= t_->names_.size()) {
    t_->id_.fetch_sub(1);
    id_ = -1;
    return;
  }
  t_->names_[id_] = name;
  t_->is_commited_[id_].store(true);
}

void Meter::Increment() {
  if (id_ == -1) {
    ++t_->storage_.PerThreadEvents()->losts;
    return;
  }
  t_->storage_.PerThreadEvents()->counters[id_] += 1;
}

} // namespace trace
} // namespace jerryct
