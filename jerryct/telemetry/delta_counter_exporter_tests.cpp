// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/counter.h"
#include "jerryct/telemetry/delta_counter_exporter.h"
#include <gtest/gtest.h>

namespace jerryct {
namespace telemetry {

TEST(DeltaCounterExporterTest, Delta) {
  Counter c{Meter(), "foo"};

  c.Add();
  c.Add();

  DeltaCounterExporter delta{};

  c.Add();
  c.Add();

  EXPECT_EQ(2, delta.Get("foo"));
}

TEST(DeltaCounterExporterTest, Reset) {
  Counter c{Meter(), "foo"};

  DeltaCounterExporter delta{};

  c.Add();
  c.Add();

  EXPECT_EQ(2, delta.Get("foo"));

  delta.Reset();

  c.Add();
  c.Add();
  c.Add();

  EXPECT_EQ(3, delta.Get("foo"));
}

} // namespace telemetry
} // namespace jerryct
