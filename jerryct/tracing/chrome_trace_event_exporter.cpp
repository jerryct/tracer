// SPDX-License-Identifier: MIT

#include "jerryct/tracing/chrome_trace_event_exporter.h"
#include <chrono>
#include <cstdio>
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

void ChromeTraceEventExporter::operator()(const std::int32_t tid, const std::uint64_t losts,
                                          const std::vector<Event> &events) {
  for (const Event &e : events) {
    const auto d = std::chrono::duration<double, std::micro>{e.time_stamp.time_since_epoch()}.count();

    if (e.phase == Phase::begin) {
      const jerryct::string_view v{e.name.Get()};
      fprintf(f_.Get(), R"({"name":"%.*s","pid":0,"tid":%d,"ph":"B","ts":%f},)", static_cast<int>(v.size()), v.data(),
              tid, d);
    }
    if (e.phase == Phase::end) {
      fprintf(f_.Get(), R"({"pid":0,"tid":%d,"ph":"E","ts":%f},)", tid, d);
    }
  }
  const auto d = std::chrono::duration<double, std::micro>{std::chrono::steady_clock::now().time_since_epoch()};
  fprintf(f_.Get(), R"({"pid":0,"name":"total lost events","ph":"C","ts":%f,"args":{"value":%lu}},)", d.count(), losts);
}

void ChromeTraceEventExporter::Rotate() {
  fprintf(f_.Get(), "{}]");
  f_.Rotate();
  fprintf(f_.Get(), "[");
}

} // namespace trace
} // namespace jerryct
