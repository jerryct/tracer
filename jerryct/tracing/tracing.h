// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_TRACING_H
#define JERRYCT_TRACING_TRACING_H

#include "jerryct/string_view.h"
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <forward_list>
#include <functional>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace jerryct {
namespace trace {

enum class Phase : std::int32_t { begin, end };

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

} // namespace trace
} // namespace jerryct

template <>
struct std::hash<jerryct::trace::FixedString> //: public std::__hash_base<size_t, jerryct::trace::FixedString>
{
  size_t operator()(const jerryct::trace::FixedString &__s) const noexcept {
    return std::_Hash_impl::hash(__s.Get().data(), __s.Get().size());
  }
};

namespace jerryct {
namespace trace {

struct Event {
  Phase phase;
  std::chrono::steady_clock::time_point time_stamp;
  FixedString name;
};

constexpr bool operator==(const FixedString &lhs, const FixedString &rhs) noexcept { return lhs.Get() == rhs.Get(); }
constexpr bool operator!=(const FixedString &lhs, const FixedString &rhs) noexcept { return !(lhs == rhs); }

template <typename T, std::uint32_t S> class LockFreeQueue {
  static_assert(std::is_trivially_destructible<T>::value, "");

  struct ManualLifetime {
    ManualLifetime() noexcept {}
    ~ManualLifetime() noexcept {}
    union {
      T value_;
    };
  };

public:
  template <typename... U> void Emplace(U &&... us) {
    const std::uint32_t ta{tail_.load(std::memory_order_acquire)};
    const std::uint32_t he{head_.load(std::memory_order_relaxed)};

    const std::uint32_t the_next{(he + 1U) % S};

    if (the_next == ta) {
      losts_.fetch_add(1U, std::memory_order_release);
      return;
    }

    new (&d_[he]) T{std::forward<U>(us)...};
    head_.store(the_next, std::memory_order_release);
  }

  template <typename F> void ConsumeAll(F &&func) {
    const std::uint32_t he{head_.load(std::memory_order_acquire)};
    std::uint32_t ta{tail_.load(std::memory_order_relaxed)};

    for (; ta != he;) {
      func(std::move(d_[ta].value_));
      ta = (ta + 1U) % S;
    }

    tail_.store(ta, std::memory_order_release);
  }

  std::uint64_t Losts() const { return losts_.load(std::memory_order_acquire); }

private:
  alignas(64) std::atomic<std::uint32_t> head_{};
  alignas(64) std::atomic<std::uint32_t> tail_{};
  alignas(64) std::atomic<std::uint64_t> losts_{};
  alignas(64) ManualLifetime d_[S];
};

class TracerImpl {
public:
  struct Events {
    std::int32_t tid;
    LockFreeQueue<Event, 4096> events;
    std::array<std::int64_t, 1024> counters;
    std::uint64_t losts;
  };

  template <typename F> void Export(F &&func) {
    std::vector<Event> v{};
    v.reserve(4096U);

    std::forward_list<Events>::iterator it;
    {
      std::lock_guard<std::mutex> guard{register_thread_};
      it = per_thread_events_.begin();
    }
    for (; it != per_thread_events_.end(); ++it) {
      v.clear();
      it->events.ConsumeAll([&v](const Event &e) { v.push_back(e); });
      func(it->tid, it->events.Losts(), static_cast<const std::vector<Event> &>(v));
    }
  }
  template <typename F> void Export2(F &&func) {
    std::vector<Event> v{};
    v.reserve(4096U);

    std::forward_list<Events>::iterator it;
    {
      std::lock_guard<std::mutex> guard{register_thread_};
      it = per_thread_events_.begin();
    }
    for (; it != per_thread_events_.end(); ++it) {
      v.clear();
      func(it->tid, names_, it->counters, s_id);
    }
  }

  Events *PerThreadEvents() {
    thread_local Events *const id{RegisterThread()};
    return id;
  }

  Events *RegisterThread() {
    std::lock_guard<std::mutex> guard{register_thread_};
    per_thread_events_.emplace_front();
    per_thread_events_.front().tid = thread_count_;
    ++thread_count_;
    return &per_thread_events_.front();
  }

  std::atomic<int> s_id{};
  std::array<FixedString, 1024> names_;

private:
  std::mutex register_thread_;
  std::int32_t thread_count_{0};
  std::forward_list<Events> per_thread_events_;
};

inline TracerImpl &Tracer() {
  static TracerImpl *const t = new TracerImpl;
  return *t;
}

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

class Meter final {
public:
  Meter(TracerImpl &t, const jerryct::string_view name);
  void Increment();

private:
  TracerImpl *t_;
  int id_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_TRACING_H
