// SPDX-License-Identifier: MIT

#ifndef TRACING_TRACING_H
#define TRACING_TRACING_H

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
    std::vector<Event> events;
  };

  std::mutex register_thread;
  std::mutex register_event;

  Events *PerThreadEvents() {
    thread_local Events *const id{RegisterThread()};
    return id;
  }

  Events *RegisterThread() {
    std::lock_guard<std::mutex> guard{register_thread};
    per_thread_events_.push_front(Events{thread_count_, {}});
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

  template <typename F> void Export(F func) {
    for (const auto &t : per_thread_events_) {
      for (const auto &e : t.events) {
        func(t.tid, attributes_[static_cast<std::size_t>(e.id)], e.p, e.ts);
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
  Span(TracerImpl &t, const Token e) : t_{&t}, id_{e.id} {
    const auto now = std::chrono::steady_clock::now();
    t_->PerThreadEvents()->events.push_back({id_, Phase::begin, now});
  }
  Span(const Span &) = delete;
  Span(Span &&) = delete;
  Span &operator=(const Span &) = delete;
  Span &operator=(Span &&) = delete;
  ~Span() { End(); }

  void End() {
    const auto now = std::chrono::steady_clock::now();
    if (id_ == -1) {
      return;
    }
    t_->PerThreadEvents()->events.push_back({id_, Phase::end, now});
    id_ = -1;
  }

private:
  TracerImpl *t_;
  int id_;
};

} // namespace trace

#endif // TRACING_TRACING_H
