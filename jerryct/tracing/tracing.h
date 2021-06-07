// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_TRACING_H
#define JERRYCT_TRACING_TRACING_H

#include "jerryct/string_view.h"
#include <atomic>
#include <chrono>
#include <cstring>
#include <forward_list>
#include <mutex>
#include <type_traits>
#include <vector>

namespace jerryct {
namespace trace {

enum class Phase { begin, end };

struct fixed_string {
  fixed_string() noexcept : s_{0} {}
  fixed_string(jerryct::string_view v) noexcept {
    if (v.empty()) {
      s_ = 0;
      return;
    }
    s_ = std::min(64UL, v.size());
    std::memmove(&d_[0], v.data(), s_);
  }

  jerryct::string_view as_string_view() const { return {&d_[0], s_}; }

  char d_[64];
  size_t s_;
};

template <typename T, std::uint32_t S> struct LockFreeQueue {
  struct ManualLifetime {
    ManualLifetime() noexcept {}
    ~ManualLifetime() noexcept {}
    union {
      T value_;
    };
  };

  alignas(64) std::atomic<std::uint32_t> head;
  alignas(64) std::atomic<std::uint32_t> tail;
  alignas(64) ManualLifetime d[S];
  static_assert(std::is_trivially_destructible<T>::value, "");

  template <typename... U> void emplace(U &&... us) {
    const std::uint32_t ta = tail.load(std::memory_order_acquire);
    std::uint32_t he = head.load(std::memory_order_relaxed);

    const std::uint32_t the_next = (he + 1U) % S;

    if (the_next == ta) {
      return;
    }

    new (&d[he]) T{std::forward<U>(us)...};
    head.store(the_next, std::memory_order_release);
  }

  template <typename F> void consume_all(F &&func) {
    const std::uint32_t he = head.load(std::memory_order_acquire);
    std::uint32_t ta = tail.load(std::memory_order_relaxed);

    for (; ta != he;) {
      func(std::move(d[ta].value_));
      ta = (ta + 1U) % S;
    }

    tail.store(ta, std::memory_order_release);
  }
};

// a thread must not outlive a tracer
// https://opentelemetry.io/docs/concepts/data-sources/
struct TracerImpl {

  struct Event {
    Phase p;
    std::chrono::steady_clock::time_point ts;
    fixed_string name;
  };

  struct Events {
    int tid;
    LockFreeQueue<Event, 4096> events;
  };

  std::mutex register_thread;

  Events *PerThreadEvents() {
    thread_local Events *const id{RegisterThread()};
    return id;
  }

  Events *RegisterThread() {
    std::lock_guard<std::mutex> guard{register_thread};
    per_thread_events_.emplace_front();
    per_thread_events_.front().tid = thread_count_;
    ++thread_count_;
    return &per_thread_events_.front();
  }

  int thread_count_{0};
  std::forward_list<Events> per_thread_events_;

  template <typename F> void Export(F &&func) {
    std::vector<Event> v;
    v.reserve(4096);

    std::forward_list<Events>::iterator it;
    {
      std::lock_guard<std::mutex> guard{register_thread};
      it = per_thread_events_.begin();
    }
    for (; it != per_thread_events_.end(); ++it) {
      v.clear();
      it->events.consume_all([&v](Event &&e) { v.push_back(std::move(e)); });
      for (const auto &e : v) {
        func(it->tid, e.name.as_string_view(), e.p, e.ts);
      }
    }
  }
};

inline TracerImpl &Tracer() {
  static TracerImpl *t = new TracerImpl;
  return *t;
}

class Span {
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

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_TRACING_H
