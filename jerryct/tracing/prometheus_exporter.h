// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_PROMETHEUS_EXPORTER_H
#define JERRYCT_TRACING_PROMETHEUS_EXPORTER_H

#include "jerryct/tracing/tracing.h"
#include <chrono>
#include <forward_list>
#include <future>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace jerryct {
namespace trace {

struct FileDesc {
  FileDesc() noexcept = default;
  FileDesc(int fd);
  FileDesc(const FileDesc &) = delete;
  FileDesc(FileDesc &&other) noexcept;
  FileDesc &operator=(const FileDesc &) = delete;
  FileDesc &operator=(FileDesc &&other) noexcept;
  ~FileDesc() noexcept;
  bool IsValid() const;

  int fd_{-1};
};

class JoinThread {
public:
  JoinThread() noexcept = default;
  template <typename Function, typename... Args>
  explicit JoinThread(int token, Function &&f, Args &&... args)
      : stop_token_{token}, t_{std::forward<Function>(f), std::forward<Args>(args)...} {}
  JoinThread(const JoinThread &) = delete;
  JoinThread &operator=(const JoinThread &) = delete;
  JoinThread(JoinThread &&other) noexcept;
  JoinThread &operator=(JoinThread &&other) noexcept;
  ~JoinThread();

private:
  int stop_token_;
  std::thread t_;
};

class PrometheusExporter;

class RequestHandler {
public:
  RequestHandler(FileDesc fd);

  void operator()(PrometheusExporter &exporter);

private:
  FileDesc fd_;
  std::vector<char> request_;
  std::string response_;
  std::string content_;
};

class Async {
public:
  template <typename Function, typename... Args>
  explicit Async(int token, Function &&f, Args &&... args)
      : p_{}, f_{p_.get_future()}, t_{token,
                                      [this](auto &&f, auto &&... args) {
                                        f(std::forward<Args>(args)...);
                                        p_.set_value();
                                      },
                                      std::forward<Function>(f), std::forward<Args>(args)...} {}

  bool HasFinished() const { return f_.wait_for(std::chrono::seconds{0}) == std::future_status::ready; }

private:
  std::promise<void> p_;
  std::future<void> f_;
  JoinThread t_;
};

class ConnectionHandler {
public:
  ConnectionHandler(PrometheusExporter &exporter);

private:
  void Await(PrometheusExporter &exporter);

  FileDesc fd_;
  std::forward_list<Async> connections_;
  JoinThread awaiter_;
};

class PrometheusExporter {
public:
  void operator()(const int tid, const std::int64_t losts, const std::vector<Event> &events);
  void Expose(std::string &content);

private:
  struct Metrics {
    std::chrono::nanoseconds min{std::chrono::nanoseconds::max()};
    std::chrono::nanoseconds max{};
    std::chrono::nanoseconds sum{};
    std::int64_t count{};
  };

  struct Frame {
    const std::string name;
    const std::chrono::steady_clock::time_point ts;
  };

  std::unordered_map<std::string, Metrics> data_;
  std::unordered_map<int, std::vector<Frame>> stacks_;
  std::unordered_map<int, std::int64_t> losts_;
  ConnectionHandler conn_{*this};
  std::mutex m_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_PROMETHEUS_EXPORTER_H
