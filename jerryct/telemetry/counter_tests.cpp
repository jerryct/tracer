// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/counter.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <thread>
#include <unordered_map>
#include <vector>

namespace jerryct {
namespace telemetry {
namespace {

TEST(CounterTest, WhenCounterCreated_ExpectCounterNameIsRegistered) {
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

TEST(CounterTest, WhenSingleThreadedCounting_ExpectAccumulatedCount) {
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

TEST(CounterTest, WhenSingleThreadedCounting_ExpectAccumulatedCountAcrossMultipleExports) {
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

    std::thread t2{[c = std::move(c)]() mutable {
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

TEST(CounterTest, WhenMultiThreadedCounting_ExpectAccumulatedCountAcrossMultipleThreads) {
  MeterImpl meter{};

  std::thread t{[&meter]() {
    Counter c{meter, "foo"};

    std::thread t1{[c]() mutable {
      for (int i{0}; i < 4096; ++i) {
        c.Add();
      }
    }};
    std::thread t2{[c = std::move(c)]() mutable {
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

TEST(CounterTest, WhenMultipleCountersWithSameName_ExpectAccumulatedCounts) {
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
} // namespace telemetry
} // namespace jerryct
