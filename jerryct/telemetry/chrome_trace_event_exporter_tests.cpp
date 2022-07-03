// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/chrome_trace_event_exporter.h"
#include <fstream>
#include <gtest/gtest.h>
#include <iterator>
#include <string>

namespace jerryct {
namespace telemetry {
namespace {

std::string Export(const std::int32_t tid, const std::uint64_t losts, const std::vector<Event> &events) {
  {
    ChromeTraceEventExporter exporter{"test.json"};
    exporter(tid, losts, events);
  }
  std::ifstream i{"test.json"};
  return {std::istreambuf_iterator<char>{i}, {}};
}

TEST(ChromeTraceEventExporterTest, ZeroTimeStampFormatting) {
  const Event event{Phase::begin, std::chrono::steady_clock::time_point{std::chrono::nanoseconds{}}, {}};
  const std::string content{Export(0, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"name":"","pid":0,"tid":0,"ph":"B","ts":0.000})"));
}

TEST(ChromeTraceEventExporterTest, SingleDigitTimeStampFormatting) {
  const Event event{Phase::begin, std::chrono::steady_clock::time_point{std::chrono::nanoseconds{9}}, {}};
  const std::string content{Export(0, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"name":"","pid":0,"tid":0,"ph":"B","ts":0.009})"));
}

TEST(ChromeTraceEventExporterTest, DoubleDigitTimeStampFormatting) {
  const Event event{Phase::begin, std::chrono::steady_clock::time_point{std::chrono::nanoseconds{99}}, {}};
  const std::string content{Export(0, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"name":"","pid":0,"tid":0,"ph":"B","ts":0.099})"));
}

TEST(ChromeTraceEventExporterTest, TripleDigitTimeStampFormatting) {
  const Event event{Phase::begin, std::chrono::steady_clock::time_point{std::chrono::nanoseconds{999}}, {}};
  const std::string content{Export(0, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"name":"","pid":0,"tid":0,"ph":"B","ts":0.999})"));
}

TEST(ChromeTraceEventExporterTest, QuadrupleDigitTimeStampFormatting) {
  const Event event{Phase::begin, std::chrono::steady_clock::time_point{std::chrono::microseconds{1}}, {}};
  const std::string content{Export(0, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"name":"","pid":0,"tid":0,"ph":"B","ts":1.000})"));
}

TEST(ChromeTraceEventExporterTest, PhaseBeginFormatting) {
  const Event event{Phase::begin, {}, {}};
  const std::string content{Export(0, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"name":"","pid":0,"tid":0,"ph":"B","ts":0.000})"));
}

TEST(ChromeTraceEventExporterTest, PhaseEndFormatting) {
  const Event event{Phase::end, {}, {}};
  const std::string content{Export(0, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"pid":0,"tid":0,"ph":"E","ts":0.000})"));
}

TEST(ChromeTraceEventExporterTest, NameFormatting) {
  const Event event{Phase::begin, {}, {"unknown"}};
  const std::string content{Export(0, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"name":"unknown","pid":0,"tid":0,"ph":"B","ts":0.000})"));
}

TEST(ChromeTraceEventExporterTest, TidFormatting) {
  const Event event{Phase::begin, {}, {}};
  const std::string content{Export(23, 0U, {event})};

  EXPECT_NE(std::string::npos, content.find(R"({"name":"","pid":0,"tid":23,"ph":"B","ts":0.000})"));
}

TEST(ChromeTraceEventExporterTest, LostsFormatting) {
  const Event event{Phase::begin, std::chrono::steady_clock::time_point{std::chrono::microseconds{42}}, {}};
  const std::string content{Export(0, 23U, {event})};

  EXPECT_NE(std::string::npos,
            content.find(R"({"pid":0,"name":"total lost events","ph":"C","ts":42.000,"args":{"value":23}})"));
}

TEST(ChromeTraceEventExporterTest, EmptyJson_WhenNoEvents) {
  const std::string content{Export(0, 0U, {})};

  EXPECT_EQ("[{}]", content);
}

TEST(ChromeTraceEventExporterTest, EmptyJson_WhenNoExport) {
  { ChromeTraceEventExporter e{"test.json"}; }
  std::ifstream i{"test.json"};
  const std::string content{std::istreambuf_iterator<char>{i}, {}};

  EXPECT_EQ("[{}]", content);
}

TEST(ChromeTraceEventExporterTest, Rotate) {
  {
    ChromeTraceEventExporter e{"test.json"};
    e.Rotate();
  }
  {
    std::ifstream i{"test.json"};
    const std::string content{std::istreambuf_iterator<char>{i}, {}};

    EXPECT_EQ("[{}]", content);
  }
  {
    std::ifstream i{"test.json1"};
    const std::string content{std::istreambuf_iterator<char>{i}, {}};

    EXPECT_EQ("[{}]", content);
  }
}

} // namespace
} // namespace telemetry
} // namespace jerryct
