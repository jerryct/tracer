// SPDX-License-Identifier: MIT

#include "jerryct/tracing/lock_free_queue.h"
#include <gtest/gtest.h>
#include <vector>

namespace jerryct {
namespace telemetry {
namespace {

TEST(LockFreeQueueTest, Emplace) {

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

} // namespace
} // namespace telemetry
} // namespace jerryct
