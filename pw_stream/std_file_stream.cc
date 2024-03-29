// Copyright 2021 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#include "pw_stream/std_file_stream.h"

namespace pw::stream {
namespace {

std::ios::seekdir WhenceToSeekDir(Stream::Whence whence) {
  switch (whence) {
    case Stream::Whence::kBeginning:
      return std::ios::beg;
    case Stream::Whence::kCurrent:
      return std::ios::cur;
    case Stream::Whence::kEnd:
      return std::ios::end;
  }
}

}  // namespace

StatusWithSize StdFileReader::DoRead(ByteSpan dest) {
  if (stream_.eof()) {
    return StatusWithSize::OutOfRange();
  }

  stream_.read(reinterpret_cast<char*>(dest.data()), dest.size());
  if (stream_.bad()) {
    return StatusWithSize::Unknown();
  }

  return StatusWithSize(stream_.gcount());
}

Status StdFileReader::DoSeek(ssize_t offset, Whence origin) {
  if (!stream_.seekg(offset, WhenceToSeekDir(origin))) {
    return Status::Unknown();
  }
  return OkStatus();
}

Status StdFileWriter::DoWrite(ConstByteSpan data) {
  if (stream_.eof()) {
    return Status::OutOfRange();
  }

  if (stream_.write(reinterpret_cast<const char*>(data.data()), data.size())) {
    return OkStatus();
  }

  return Status::Unknown();
}

Status StdFileWriter::DoSeek(ssize_t offset, Whence origin) {
  if (!stream_.seekp(offset, WhenceToSeekDir(origin))) {
    return Status::Unknown();
  }
  return OkStatus();
}

}  // namespace pw::stream
