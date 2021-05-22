// SPDX-License-Identifier: MIT

#include "tracing/tracing.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace trace {
namespace {

TEST(TracerTest, DurationEvent) {
  TracerImpl tracer{};

  const Token m{tracer.RegisterDurationEvent("main", "duration")};
  const Token foo{tracer.RegisterDurationEvent("foo", "duration")};
  const Token bar{tracer.RegisterDurationEvent("bar", "duration")};
  const Token baz{tracer.RegisterDurationEvent("baz", "duration")};

  std::thread t{[m, foo, bar, baz, &tracer]() {
    Span s1{tracer, m};
    {
      Span s2{tracer, foo};
      { Span s3{tracer, bar}; }
      { Span s4{tracer, baz}; }
    }
  }};
  t.join();

  std::vector<std::tuple<std::string, Phase>> events{};
  std::vector<std::chrono::steady_clock::time_point> time_stamps{};

  tracer.Export([&events, &time_stamps](int, Attributes a, Phase p, std::chrono::steady_clock::time_point ts) {
    if ((a.tag == "duration") && (a.type == Type::duration)) {
      events.emplace_back(a.name, p);
      time_stamps.emplace_back(ts);
    }
  });

  ASSERT_EQ(8U, events.size());

  EXPECT_EQ(std::make_tuple("main", Phase::begin), events[0U]);
  EXPECT_EQ(std::make_tuple("foo", Phase::begin), events[1U]);
  EXPECT_EQ(std::make_tuple("bar", Phase::begin), events[2U]);
  EXPECT_EQ(std::make_tuple("bar", Phase::end), events[3U]);
  EXPECT_EQ(std::make_tuple("baz", Phase::begin), events[4U]);
  EXPECT_EQ(std::make_tuple("baz", Phase::end), events[5U]);
  EXPECT_EQ(std::make_tuple("foo", Phase::end), events[6U]);
  EXPECT_EQ(std::make_tuple("main", Phase::end), events[7U]);

  EXPECT_TRUE(std::is_sorted(time_stamps.cbegin(), time_stamps.cend()));
}

TEST(TracerTest, AsyncEvent) {
  TracerImpl tracer{};

  const Token m{tracer.RegisterAsyncEvent("main", "async")};
  const Token foo{tracer.RegisterAsyncEvent("foo", "async")};
  const Token bar{tracer.RegisterAsyncEvent("bar", "async")};
  const Token baz{tracer.RegisterAsyncEvent("baz", "async")};

  std::thread t{[m, foo, bar, baz, &tracer]() {
    Span s1{tracer, m};
    {
      Span s2{tracer, foo};
      { Span s3{tracer, bar}; }
      { Span s4{tracer, baz}; }
    }
  }};
  t.join();

  std::vector<std::tuple<std::string, Phase>> events{};
  std::vector<std::chrono::steady_clock::time_point> time_stamps{};

  tracer.Export([&events, &time_stamps](int, Attributes a, Phase p, std::chrono::steady_clock::time_point ts) {
    if ((a.tag == "async") && (a.type == Type::async)) {
      events.emplace_back(a.name, p);
      time_stamps.emplace_back(ts);
    }
  });

  ASSERT_EQ(8U, events.size());

  EXPECT_EQ(std::make_tuple("main", Phase::begin), events[0U]);
  EXPECT_EQ(std::make_tuple("foo", Phase::begin), events[1U]);
  EXPECT_EQ(std::make_tuple("bar", Phase::begin), events[2U]);
  EXPECT_EQ(std::make_tuple("bar", Phase::end), events[3U]);
  EXPECT_EQ(std::make_tuple("baz", Phase::begin), events[4U]);
  EXPECT_EQ(std::make_tuple("baz", Phase::end), events[5U]);
  EXPECT_EQ(std::make_tuple("foo", Phase::end), events[6U]);
  EXPECT_EQ(std::make_tuple("main", Phase::end), events[7U]);

  EXPECT_TRUE(std::is_sorted(time_stamps.cbegin(), time_stamps.cend()));
}

TEST(TracerTest, Threaded) {
  TracerImpl tracer{};

  const Token thread{tracer.RegisterDurationEvent("main", "threaded")};

  std::thread t1{[thread, &tracer]() { Span s1{tracer, thread}; }};
  std::thread t2{[thread, &tracer]() { Span s1{tracer, thread}; }};
  t1.join();
  t2.join();

  std::vector<std::tuple<std::string, Phase>> events{};
  std::vector<int> tids{};

  tracer.Export([&events, &tids](int tid, Attributes a, Phase p, std::chrono::steady_clock::time_point) {
    if (a.tag == "threaded") {
      events.emplace_back(a.name, p);
      tids.emplace_back(tid);
    }
  });

  ASSERT_EQ(4U, events.size());

  EXPECT_EQ(std::make_tuple("main", Phase::begin), events[0U]);
  EXPECT_EQ(std::make_tuple("main", Phase::end), events[1U]);
  EXPECT_EQ(std::make_tuple("main", Phase::begin), events[2U]);
  EXPECT_EQ(std::make_tuple("main", Phase::end), events[3U]);

  ASSERT_EQ(4U, tids.size());

  EXPECT_EQ(tids[0U], tids[1U]);
  EXPECT_EQ(tids[2U], tids[3U]);
  EXPECT_NE(tids[0U], tids[2U]);
}

} // namespace
} // namespace trace
