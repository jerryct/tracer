// SPDX-License-Identifier: MIT

#include "jerryct/tracing/tracing.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace jerryct {
namespace trace {
namespace {

TEST(TracerTest, SingleThread) {
  TracerImpl tracer{};

  std::thread t{[&tracer]() {
    Span s1{tracer, "main"};
    {
      Span s2{tracer, "foo"};
      { Span s3{tracer, "bar"}; }
      { Span s4{tracer, "baz"}; }
    }
  }};
  t.join();

  std::vector<std::tuple<std::string, Phase>> events{};
  std::vector<std::chrono::steady_clock::time_point> time_stamps{};

  tracer.Export([&events, &time_stamps](const std::int32_t /*unused*/, const std::uint64_t losts,
                                        const std::vector<Event> &data) {
    EXPECT_EQ(0U, losts);
    for (const Event &e : data) {
      events.emplace_back(std::string{e.name.Get().data(), e.name.Get().size()}, e.phase);
      time_stamps.emplace_back(e.time_stamp);
    }
  });

  ASSERT_EQ(8U, events.size());

  EXPECT_EQ(std::make_tuple("main", Phase::begin), events[0U]);
  EXPECT_EQ(std::make_tuple("foo", Phase::begin), events[1U]);
  EXPECT_EQ(std::make_tuple("bar", Phase::begin), events[2U]);
  EXPECT_EQ(std::make_tuple("", Phase::end), events[3U]);
  EXPECT_EQ(std::make_tuple("baz", Phase::begin), events[4U]);
  EXPECT_EQ(std::make_tuple("", Phase::end), events[5U]);
  EXPECT_EQ(std::make_tuple("", Phase::end), events[6U]);
  EXPECT_EQ(std::make_tuple("", Phase::end), events[7U]);

  EXPECT_TRUE(std::is_sorted(time_stamps.cbegin(), time_stamps.cend()));
}

TEST(TracerTest, MultiThreaded) {
  TracerImpl tracer{};

  std::thread t1{[&tracer]() { Span s1{tracer, "main"}; }};
  std::thread t2{[&tracer]() { Span s1{tracer, "main"}; }};
  t1.join();
  t2.join();

  std::vector<std::tuple<std::string, Phase>> events{};
  std::vector<std::int32_t> tids{};

  tracer.Export([&events, &tids](const std::int32_t tid, const std::uint64_t losts, const std::vector<Event> &data) {
    EXPECT_EQ(0U, losts);
    for (const Event &e : data) {
      events.emplace_back(std::string{e.name.Get().data(), e.name.Get().size()}, e.phase);
      tids.emplace_back(tid);
    }
  });

  ASSERT_EQ(4U, events.size());

  EXPECT_EQ(std::make_tuple("main", Phase::begin), events[0U]);
  EXPECT_EQ(std::make_tuple("", Phase::end), events[1U]);
  EXPECT_EQ(std::make_tuple("main", Phase::begin), events[2U]);
  EXPECT_EQ(std::make_tuple("", Phase::end), events[3U]);

  ASSERT_EQ(4U, tids.size());

  EXPECT_EQ(tids[0U], tids[1U]);
  EXPECT_EQ(tids[2U], tids[3U]);
  EXPECT_NE(tids[0U], tids[2U]);
}

TEST(TracerTest, LockFreeQueue) {

  LockFreeQueue<std::int32_t, 4> r{};

  {
    std::vector<std::int32_t> o;
    o.reserve(4U);
    r.ConsumeAll([&o](const std::int32_t v) { o.push_back(v); });
    ASSERT_EQ(0U, o.size());
  }

  r.Emplace(1);
  r.Emplace(2);
  r.Emplace(3);
  r.Emplace(4);

  {
    std::vector<std::int32_t> o;
    o.reserve(4U);
    r.ConsumeAll([&o](const std::int32_t v) { o.push_back(v); });
    ASSERT_EQ(3U, o.size());
    EXPECT_EQ(1U, r.Losts());
    EXPECT_EQ(1, o[0U]);
    EXPECT_EQ(2, o[1U]);
    EXPECT_EQ(3, o[2U]);
  }

  r.Emplace(5);
  r.Emplace(6);
  r.Emplace(7);
  r.Emplace(8);

  {
    std::vector<std::int32_t> o;
    o.reserve(4U);
    r.ConsumeAll([&o](const std::int32_t v) { o.push_back(v); });
    ASSERT_EQ(3U, o.size());
    EXPECT_EQ(2U, r.Losts());
    EXPECT_EQ(5, o[0U]);
    EXPECT_EQ(6, o[1U]);
    EXPECT_EQ(7, o[2U]);
  }
}

struct Foo {
  Foo() { printf("ctor\n"); }
  Foo(const Foo & /*unused*/) { printf("copy\n"); }
  Foo(Foo && /*unused*/) { printf("move\n"); }
  Foo &operator=(const Foo & /*unused*/) {
    printf("copyassign\n");
    return *this;
  }
  Foo &operator=(Foo && /*unused*/) {
    printf("moveassign\n");
    return *this;
  }
  //~Foo() { printf("dtor\n"); }
};

TEST(TracerTest, ASDF) {

  LockFreeQueue<Foo, 4> r{};

  r.Emplace();
  r.Emplace();
  r.Emplace();
  r.Emplace();
}

} // namespace
} // namespace trace
} // namespace jerryct
