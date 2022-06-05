// SPDX-License-Identifier: MIT

#include "jerryct/tracing/tracing.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <thread>
#include <unordered_map>
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

TEST(Meter, Counter_WhenCounterCreated_ExpectCounterNameIsRegistered) {
  MeterImpl meter{};

  std::thread t{[&meter]() {
    std::thread t1{[&meter]() {
      const Counter c1{meter, "foo1"};
      const Counter c2{meter, "foo2"};
    }};
    std::thread t2{[&meter]() {
      const Counter c1{meter, "bar1"};
      const Counter c2{meter, "bar2"};
    }};
    t1.join();
    t2.join();
  }};
  t.join();

  meter.Export([](const std::unordered_map<string_view, std::uint64_t> &data) {
    const std::vector<std::unordered_map<string_view, std::uint64_t>::value_type> expected{
        std::make_pair(string_view{"measurement_losts"}, 0), std::make_pair(string_view{"foo1"}, 0),
        std::make_pair(string_view{"foo2"}, 0), std::make_pair(string_view{"bar1"}, 0),
        std::make_pair(string_view{"bar2"}, 0)};
    EXPECT_TRUE(std::is_permutation(data.cbegin(), data.cend(), expected.cbegin(), expected.cend()));
  });
}

TEST(Meter, Counter_WhenSingleThreadedCounting_ExpectAccumulatedCount) {
  MeterImpl meter{};

  std::thread t1{[&meter]() {
    Counter c{meter, "foo"};

    std::thread t2{[c]() mutable {
      for (int i{0}; i < 4096; ++i) {
        c.Add();
      }
    }};
    t2.join();
  }};
  t1.join();

  meter.Export([](const std::unordered_map<string_view, std::uint64_t> &data) {
    const std::vector<std::unordered_map<string_view, std::uint64_t>::value_type> expected{
        std::make_pair(string_view{"measurement_losts"}, 1), std::make_pair(string_view{"foo"}, 4095)};
    EXPECT_TRUE(std::is_permutation(data.cbegin(), data.cend(), expected.cbegin(), expected.cend()));
  });
}

TEST(Meter, Counter_WhenSingleThreadedCounting_ExpectAccumulatedCountAcrossMultipleExports) {
  MeterImpl meter{};

  std::thread t{[&meter]() {
    Counter c{meter, "foo"};

    std::thread t1{[c]() mutable {
      for (int i{0}; i < 2000; ++i) {
        c.Add();
      }
    }};
    t1.join();

    meter.Export([](const std::unordered_map<string_view, std::uint64_t> &) {});

    std::thread t2{[c]() mutable {
      for (int i{0}; i < 1000; ++i) {
        c.Add();
      }
    }};
    t2.join();
  }};
  t.join();

  meter.Export([](const std::unordered_map<string_view, std::uint64_t> &data) {
    const std::vector<std::unordered_map<string_view, std::uint64_t>::value_type> expected{
        std::make_pair(string_view{"measurement_losts"}, 0), std::make_pair(string_view{"foo"}, 3000)};
    EXPECT_TRUE(std::is_permutation(data.cbegin(), data.cend(), expected.cbegin(), expected.cend()));
  });
}

TEST(Meter, Counter_WhenMultiThreadedCounting_ExpectAccumulatedCountAcrossMultipleThreads) {
  MeterImpl meter{};

  std::thread t{[&meter]() {
    Counter c{meter, "foo"};

    std::thread t1{[c]() mutable {
      for (int i{0}; i < 4096; ++i) {
        c.Add();
      }
    }};
    std::thread t2{[&c]() {
      for (int i{0}; i < 4097; ++i) {
        c.Add();
      }
    }};
    t1.join();
    t2.join();
  }};
  t.join();

  meter.Export([](const std::unordered_map<string_view, std::uint64_t> &data) {
    const std::vector<std::unordered_map<string_view, std::uint64_t>::value_type> expected{
        std::make_pair(string_view{"measurement_losts"}, 3), std::make_pair(string_view{"foo"}, 8190)};
    EXPECT_TRUE(std::is_permutation(data.cbegin(), data.cend(), expected.cbegin(), expected.cend()));
  });
}

TEST(Meter, Counter_WhenMultipleCountersWithSameName_ExpectAccumulatedCounts) {
  MeterImpl meter{};

  std::thread t{[&meter]() {
    std::thread t1{[&meter]() {
      Counter c{meter, "foo"};
      for (int i{0}; i < 1000; ++i) {
        c.Add();
      }
    }};
    std::thread t2{[&meter]() {
      Counter c{meter, "foo"};
      for (int i{0}; i < 3000; ++i) {
        c.Add();
      }
    }};
    t1.join();
    t2.join();
  }};
  t.join();

  meter.Export([](const std::unordered_map<string_view, std::uint64_t> &data) {
    const std::vector<std::unordered_map<string_view, std::uint64_t>::value_type> expected{
        std::make_pair(string_view{"measurement_losts"}, 0), std::make_pair(string_view{"foo"}, 4000)};
    EXPECT_TRUE(std::is_permutation(data.cbegin(), data.cend(), expected.cbegin(), expected.cend()));
  });
}

} // namespace
} // namespace trace
} // namespace jerryct
