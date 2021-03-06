// Copyright 2020 The Pigweed Authors
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

#include "pw_tokenizer/tokenize_to_global_handler.h"

#include "pw_tokenizer_private/encode_args.h"

namespace pw {
namespace tokenizer {

extern "C" void _pw_TokenizeToGlobalHandler(pw_TokenizerStringToken token,
                                            pw_TokenizerArgTypes types,
                                            ...) {
  EncodedMessage encoded;
  encoded.token = token;

  va_list args;
  va_start(args, types);
  const size_t encoded_bytes = EncodeArgs(types, args, encoded.args);
  va_end(args);

  pw_TokenizerHandleEncodedMessage(reinterpret_cast<const uint8_t*>(&encoded),
                                   sizeof(encoded.token) + encoded_bytes);
}

}  // namespace tokenizer
}  // namespace pw
