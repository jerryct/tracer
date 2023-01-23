// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_LOCK_FREE_QUEUE_H
#define JERRYCT_TELEMETRY_LOCK_FREE_QUEUE_H

#include <atomic>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace jerryct {
namespace telemetry {

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
      losts_.store(losts_.load(std::memory_order_relaxed) + 1U, std::memory_order_relaxed);
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

  std::uint64_t Losts() const { return losts_.load(std::memory_order_relaxed); }

private:
  alignas(64) std::atomic<std::uint32_t> head_{};
  alignas(64) std::atomic<std::uint32_t> tail_{};
  alignas(64) std::atomic<std::uint64_t> losts_{};
  alignas(64) ManualLifetime d_[S];
};

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_LOCK_FREE_QUEUE_H
