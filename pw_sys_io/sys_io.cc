// Copyright 2019 The Pigweed Authors
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

#include "pw_sys_io/sys_io.h"

namespace pw::sys_io {

StatusWithSize ReadBytes(std::span<std::byte> dest) { return Receive(dest); }

StatusWithSize WriteBytes(std::span<const std::byte> src) {
  return Transmit(src);
}

}  // namespace pw::sys_io
