// SPDX-License-Identifier: MIT

#ifndef TRACING_TRACING_H
#define TRACING_TRACING_H

#include <atomic>
#include <chrono>
#include <forward_list>
#include <mutex>
#include <string>
#include <vector>

namespace trace {

struct Token {
  int id;
};

enum class Phase { begin, end };
enum class Type { duration, async };

struct Attributes {
  std::string name;
  std::string tag;
  Type type;
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

  template <typename F> void consumer_all(F &&func) {
    const std::uint32_t he = head.load(std::memory_order_acquire);
    std::uint32_t ta = tail.load(std::memory_order_relaxed);

    for (; ta != he;) {
      func(std::move(d[ta].value_));
      ta = (ta + 1U) % S;
    }

    tail.store(ta, std::memory_order_release);
  }

  template <typename Iter> void pop_all(Iter it) {
    const std::uint32_t he = head.load(std::memory_order_acquire);
    std::uint32_t ta = tail.load(std::memory_order_relaxed);

    for (; ta != he;) {
      *it = std::move(d[ta].value_);
      ++it;
      ta = (ta + 1U) % S;
    }

    tail.store(ta, std::memory_order_release);
  }
};

// a thread must not outlive a tracer
// https://opentelemetry.io/docs/concepts/data-sources/
struct TracerImpl {

  struct Event {
    int id;
    Phase p;
    std::chrono::steady_clock::time_point ts;
  };

  struct Events {
    int tid;
    LockFreeQueue<Event, 4096> events;
  };

  std::mutex register_thread;
  std::mutex register_event;

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

  Token RegisterDurationEvent(std::string name, std::string tag) {
    std::lock_guard<std::mutex> guard{register_event};
    Token e{static_cast<int>(attributes_.size())};
    attributes_.push_back({name, tag, Type::duration});
    return e;
  }

  Token RegisterAsyncEvent(std::string name, std::string tag) {
    std::lock_guard<std::mutex> guard{register_event};
    Token e{static_cast<int>(attributes_.size())};
    attributes_.push_back({name, tag, Type::async});
    return e;
  }

  std::vector<Attributes> attributes_;

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
      it->events.pop_all(std::back_inserter(v));
      for (const auto &e : v) {
        func(it->tid, attributes_[static_cast<std::size_t>(e.id)], e.p, e.ts);
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
  Span(TracerImpl &t, const Token e) : t_{t.PerThreadEvents()}, id_{e.id} {
    const auto now = std::chrono::steady_clock::time_point{}; // std::chrono::steady_clock::now();
    t_->events.emplace(id_, Phase::begin, now);
  }
  Span(const Span &) = delete;
  Span(Span &&) = delete;
  Span &operator=(const Span &) = delete;
  Span &operator=(Span &&) = delete;
  ~Span() noexcept { End(); }

  void End() {
    const auto now = std::chrono::steady_clock::time_point{}; // std::chrono::steady_clock::now();
    if (nullptr == t_) {
      return;
    }
    t_->events.emplace(id_, Phase::end, now);
    t_ = nullptr;
  }

private:
  TracerImpl::Events *t_;
  int id_;
};

} // namespace trace

#endif // TRACING_TRACING_H
