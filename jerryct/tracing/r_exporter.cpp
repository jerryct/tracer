// SPDX-License-Identifier: MIT

#include "jerryct/tracing/r_exporter.h"
#include <algorithm>
#include <cstdio>
#include <numeric>

// https://cran.r-project.org/doc/manuals/r-patched/R-ints.html#Serialization-Formats
// https://yetanothermathprogrammingconsultant.blogspot.com/2016/02/r-rdata-file-format.html
// https://www.loc.gov/preservation/digital/formats/fdd/fdd000470.shtml
// > load("export.rdata")
// > hist(i, xlim=c(0.00015,0.00025), breaks= c(seq(0.00015,0.00025,by=0.000001),10))
// $ hexdump -C export.rdata

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

#pragma pack(push, 1)

struct rdata_v2_header_t {
  char header[2];
  uint32_t format_version;
  uint32_t writer_version;
  uint32_t reader_version;
};

#pragma pack(pop)

struct rdata_sexptype_header_t {
  unsigned int type : 8;
  unsigned int object : 1;
  unsigned int attributes : 1;
  unsigned int tag : 1;
  unsigned int unused : 1;
  unsigned int gp : 16;
  unsigned int padding : 4;
};

void BeginFile(int fd) {
  write(fd, "RDX2\n", 5);

  rdata_v2_header_t v2_header;
  v2_header.header[0] = 'B';
  v2_header.header[1] = '\n';
  v2_header.format_version = 2;
  v2_header.reader_version = 197636;
  v2_header.writer_version = 131840;

  write(fd, &v2_header, sizeof(v2_header));
}

void EndFile(int fd) {
  rdata_sexptype_header_t header{};
  header.type = 254; // PSEUDO_SXP_NIL
  write(fd, &header, sizeof(header));
}

void Serialize(int fd, const std::string &n, const std::vector<double> &b) {
  { // LISTSXP object: whole thing is packaged in a dotted pair list
    const unsigned v = 1026;
    write(fd, &v, sizeof(v));
  }
  { // SYMSXP object: symbol
    const unsigned v = 1;
    write(fd, &v, sizeof(v));
  }
  { // CHARSXP object: string
    const unsigned v = 262153;
    write(fd, &v, sizeof(v));
  }
  { // Length of string
    const int v = static_cast<int>(n.size());
    write(fd, &v, sizeof(v));
  }
  { // String: symbol name
    write(fd, n.data(), n.size());
  }
  { // REALSXP: real vector
    const unsigned v = 14;
    write(fd, &v, sizeof(v));
  }
  { // Length of vector
    const int v = static_cast<int>(b.size());
    write(fd, &v, sizeof(v));
  }
  { // elements
    write(fd, b.data(), b.size() * sizeof(double));
  }
}

} // namespace

namespace jerryct {
namespace trace {

RExporter::RExporter(const std::string &filename) : fd_{open(filename.c_str(), O_CREAT | O_WRONLY, 0644)} {
  if (-1 == fd_) {
    throw;
  }
}

RExporter::RExporter(RExporter &&other) noexcept : fd_{other.fd_} { other.fd_ = -1; }

RExporter &RExporter::operator=(RExporter &&other) noexcept {
  if (fd_ != other.fd_) {
    std::swap(fd_, other.fd_);
  }
  return *this;
}

RExporter::~RExporter() noexcept {
  if (-1 == fd_) {
    return;
  }

  BeginFile(fd_);

  for (auto &itt : data_) {
    Serialize(fd_, itt.first, itt.second);
  }

  EndFile(fd_);
  close(fd_);
}

void RExporter::operator()(const int tid, const std::vector<Event> &events) {
  for (const Event &e : events) {
    switch (e.p) {
    case Phase::begin:
      stacks_[tid].push_back({{e.name.get().data(), e.name.get().size()}, e.ts});
      break;
    case Phase::end:
      if (!stacks_[tid].empty()) {
        const auto d = std::chrono::duration<double, std::milli>{e.ts - stacks_[tid].back().ts}.count();
        data_[stacks_[tid].back().name].push_back(d);
        stacks_[tid].pop_back();
      }
      break;
    }
  }
}

} // namespace trace
} // namespace jerryct
