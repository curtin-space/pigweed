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
#pragma once

#include "pw_trace_protos/trace_rpc.rpc.pb.h"

namespace pw::trace {

class TraceService final : public generated::TraceService<TraceService> {
 public:
  pw::Status Enable(const pw_trace_TraceEnableMessage& request,
                    pw_trace_TraceEnableMessage& response);

  pw::Status IsEnabled(const pw_trace_Empty& request,
                       pw_trace_TraceEnableMessage& response);

  void GetTraceData(const pw_trace_Empty& request,
                    ServerWriter<pw_trace_TraceDataMessage>& writer);
};

}  // namespace pw::trace
