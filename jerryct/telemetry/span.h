// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_SPAN_H
#define JERRYCT_TELEMETRY_SPAN_H

#include "jerryct/string_view.h"
#include "jerryct/telemetry/tracer.h"

namespace jerryct {
namespace telemetry {

class Span final {
public:
  Span(TracerImpl &t, const jerryct::string_view name);
  Span(const Span &) = delete;
  Span(Span &&) = delete;
  Span &operator=(const Span &) = delete;
  Span &operator=(Span &&) = delete;
  ~Span() noexcept;

private:
  TracerImpl::Events *t_;
};

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_SPAN_H
