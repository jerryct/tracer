// SPDX-License-Identifier: MIT

#include "jerryct/tracing/chrome_trace_event_exporter.h"
#include <chrono>
#include <cstdio>
#include <fmt/core.h>
#include <string>

namespace jerryct {
namespace trace {

File::File(const std::string &filename) : f_{::fopen(filename.c_str(), "w")} {
  if (!IsValid()) {
    throw;
  }
}

File::File(File &&other) noexcept : f_{other.f_} { other.f_ = nullptr; }

File &File::operator=(File &&other) noexcept {
  if (f_ != other.f_) {
    std::swap(f_, other.f_);
  }
  return *this;
}

File::~File() noexcept {
  if (IsValid()) {
    ::fclose(f_);
  }
}
bool File::IsValid() const { return f_ != nullptr; }

FileRotate::FileRotate(const std::string &filename) : f_{}, filename_{filename} { Rotate(); }

void FileRotate::Rotate() {
  ::remove((filename_ + std::to_string(rotation_size - 1)).c_str());
  for (int i{rotation_size - 1}; i > 1; --i) {
    ::rename((filename_ + std::to_string(i - 1)).c_str(), (filename_ + std::to_string(i)).c_str());
  }
  ::rename(filename_.c_str(), (filename_ + std::to_string(1)).c_str());

  f_ = {filename_};
}

std::FILE *FileRotate::Get() const { return f_.f_; }

ChromeTraceEventExporter::ChromeTraceEventExporter(const std::string &filename) : f_{filename} {
  fprintf(f_.Get(), "[");
}

ChromeTraceEventExporter::~ChromeTraceEventExporter() noexcept { fprintf(f_.Get(), "{}]"); }

namespace {

void FormatAsMicro(const std::chrono::steady_clock::time_point tp, fmt::memory_buffer &buf) {
  const auto micro = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch());
  buf.append(fmt::format_int{micro.count()});
  buf.push_back('.');
  const auto nano = (tp.time_since_epoch() - micro).count();
  if (nano < 100) {
    buf.push_back('0');
  }
  if (nano < 10) {
    buf.push_back('0');
  }
  buf.append(fmt::format_int{nano});
}

} // namespace

void ChromeTraceEventExporter::operator()(const std::int32_t tid, const std::uint64_t losts,
                                          const std::vector<Event> &events) {
  for (const Event &e : events) {
    switch (e.phase) {
    case Phase::begin:
      buf_.append(fmt::string_view{R"({"name":")"});
      buf_.append(e.name.Get());
      buf_.append(fmt::string_view{R"(","pid":0,"tid":)"});
      buf_.append(fmt::format_int{tid});
      buf_.append(fmt::string_view{R"(,"ph":"B","ts":)"});
      FormatAsMicro(e.time_stamp, buf_);
      buf_.append(fmt::string_view{R"(},)"});
      break;
    case Phase::end:
      buf_.append(fmt::string_view{R"({"pid":0,"tid":)"});
      buf_.append(fmt::format_int{tid});
      buf_.append(fmt::string_view{R"(,"ph":"E","ts":)"});
      FormatAsMicro(e.time_stamp, buf_);
      buf_.append(fmt::string_view{R"(},)"});
      break;
    }
  }

  buf_.append(fmt::string_view{R"({"pid":0,"name":"total lost events","ph":"C","ts":)"});
  FormatAsMicro(std::chrono::steady_clock::now(), buf_);
  buf_.append(fmt::string_view{R"(,"args":{"value":)"});
  buf_.append(fmt::format_int{losts});
  buf_.append(fmt::string_view{R"(}},)"});

  std::fwrite(buf_.data(), 1, buf_.size(), f_.Get());
  buf_.clear();
}

void ChromeTraceEventExporter::Rotate() {
  fprintf(f_.Get(), "{}]");
  f_.Rotate();
  fprintf(f_.Get(), "[");
}

} // namespace trace
} // namespace jerryct
