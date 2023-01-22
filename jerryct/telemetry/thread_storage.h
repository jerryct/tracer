// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_THREAD_STORAGE_H
#define JERRYCT_TELEMETRY_THREAD_STORAGE_H

#include <cstdint>
#include <forward_list>
#include <memory>
#include <mutex>

namespace jerryct {
namespace telemetry {

template <typename T> class ThreadStorage {
  struct Content {
    std::int32_t tid;
    T data;
  };

public:
  template <typename F> void Export(F &&func) {
    typename std::forward_list<std::shared_ptr<Content>>::iterator it;
    {
      std::lock_guard<std::mutex> guard{register_thread_};
      it = per_thread_events_.begin();
    }
    for (; it != per_thread_events_.end(); ++it) {
      Content *const c{it->get()};
      func(c->tid, c->data);
    }
  }

  T *PerThreadEvents() {
    thread_local std::shared_ptr<Content> id{RegisterThread()};
    return &id->data;
  }

  std::shared_ptr<Content> RegisterThread() {
    std::lock_guard<std::mutex> guard{register_thread_};
    per_thread_events_.push_front(std::make_unique<Content>());
    per_thread_events_.front()->tid = thread_count_;
    ++thread_count_;
    return per_thread_events_.front();
  }

private:
  std::mutex register_thread_;
  std::int32_t thread_count_{0};
  std::forward_list<std::shared_ptr<Content>> per_thread_events_;
};

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_THREAD_STORAGE_H
