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
#include <mutex>
#include <set>
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

constexpr bool operator==(const FixedString &lhs, const FixedString &rhs) noexcept { return lhs.Get() == rhs.Get(); }
constexpr bool operator<(const FixedString &lhs, const FixedString &rhs) noexcept { return lhs.Get() < rhs.Get(); }

struct Event {
  Phase phase;
  std::chrono::steady_clock::time_point time_stamp;
  FixedString name;
};

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
      losts_.store(losts_.load(std::memory_order_relaxed) + 1U, std::memory_order_release);
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

template <typename T> class ThreadStorage {
public:
  template <typename F> void Export(F &&func) {
    typename std::forward_list<Content>::iterator it;
    {
      std::lock_guard<std::mutex> guard{register_thread_};
      it = per_thread_events_.begin();
    }
    for (; it != per_thread_events_.end(); ++it) {
      func(it->tid, it->data);
    }
  }

  T *PerThreadEvents() {
    thread_local T *const id{RegisterThread()};
    return id;
  }

  T *RegisterThread() {
    std::lock_guard<std::mutex> guard{register_thread_};
    per_thread_events_.emplace_front();
    per_thread_events_.front().tid = thread_count_;
    ++thread_count_;
    return &per_thread_events_.front().data;
  }

private:
  struct Content {
    std::int32_t tid;
    T data;
  };

  std::mutex register_thread_;
  std::int32_t thread_count_{0};
  std::forward_list<Content> per_thread_events_;
};

class TracerImpl {
public:
  struct Events {
    LockFreeQueue<Event, 4096> events;
  };

  template <typename F> void Export(F &&func) {
    std::vector<Event> v{};
    v.reserve(4096U);

    storage_.Export([&v, &func](std::int32_t tid, Events &e) {
      v.clear();
      e.events.ConsumeAll([&v](const Event &e) { v.push_back(e); });
      func(tid, e.events.Losts(), static_cast<const std::vector<Event> &>(v));
    });
  }

  Events *PerThreadEvents() { return storage_.PerThreadEvents(); }

private:
  ThreadStorage<Events> storage_;
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

struct Measurement {
  std::uint64_t value;
  const FixedString *id;
};

class MeterImpl {
public:
  struct Measurements {
    LockFreeQueue<Measurement, 4096> events;
  };

  MeterImpl() { RegisterName("measurement_losts"); }

  template <typename F> void Export(F &&func) {
    std::uint64_t &total_losts{counters_[string_view{"measurement_losts"}]};
    total_losts = 0;

    storage_.Export([&total_losts, &v = counters_](std::int32_t /*unused*/, Measurements &e) {
      total_losts += e.events.Losts();
      e.events.ConsumeAll([&v](const Measurement &m) { v[m.id->Get()] += m.value; });
    });

    func(static_cast<const std::unordered_map<string_view, std::uint64_t> &>(counters_));
  }

  Measurements *PerThreadEvents() { return storage_.PerThreadEvents(); }

  const FixedString *RegisterName(const string_view name) {
    std::lock_guard<std::mutex> guard{register_names_};
    return &(*names_.emplace(name).first);
  }

private:
  std::mutex register_names_;
  std::set<FixedString> names_;
  std::unordered_map<string_view, std::uint64_t> counters_;
  ThreadStorage<Measurements> storage_;
};

inline MeterImpl &Meter() {
  static MeterImpl *const t = new MeterImpl;
  return *t;
}

class Counter final {
public:
  Counter(MeterImpl &t, const jerryct::string_view name);
  Counter(const Counter &) = default;
  Counter(Counter &&) = delete;
  Counter &operator=(const Counter &) = default;
  Counter &operator=(Counter &&) = delete;
  ~Counter() noexcept = default;

  void Add();
  void Add(const std::int64_t v);

private:
  MeterImpl *t_;
  const FixedString *id_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_TRACING_H
