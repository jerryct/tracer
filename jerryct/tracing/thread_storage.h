// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_THREAD_STORAGE_H
#define JERRYCT_TELEMETRY_THREAD_STORAGE_H

#include <cstdint>
#include <forward_list>
#include <mutex>

namespace jerryct {
namespace telemetry {

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

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_THREAD_STORAGE_H
