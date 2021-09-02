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
#pragma once

#include "pw_bytes/span.h"
#include "pw_rpc/internal/method.h"
#include "pw_rpc/internal/responder.h"
#include "pw_rpc/method_type.h"
#include "pw_rpc/raw/server_reader_writer.h"
#include "pw_status/status_with_size.h"

namespace pw::rpc::internal {

// A RawMethod is a method invoker which does not perform any automatic protobuf
// serialization or deserialization. The implementer is given the raw binary
// payload of incoming requests, and is responsible for encoding responses to a
// provided buffer. This is intended for use in methods which would have large
// protobuf data structure overhead to lower stack usage, or in methods packing
// responses up to a channel's MTU.
class RawMethod : public Method {
 public:
  template <auto kMethod>
  static constexpr bool matches() {
    return std::is_same_v<MethodImplementation<kMethod>, RawMethod>;
  }

  template <auto kMethod>
  static constexpr RawMethod Unary(uint32_t id) {
    constexpr SynchronousUnaryFunction wrapper =
        [](CallContext& call, ConstByteSpan req, ByteSpan res) {
          return CallMethodImplFunction<kMethod>(call, req, res);
        };
    return RawMethod(
        id, SynchronousUnaryInvoker, Function{.synchronous_unary = wrapper});
  }

  template <auto kMethod>
  static constexpr RawMethod ServerStreaming(uint32_t id) {
    constexpr UnaryRequestFunction wrapper =
        [](CallContext& call, ConstByteSpan request, RawServerWriter& writer) {
          return CallMethodImplFunction<kMethod>(call, request, writer);
        };
    return RawMethod(
        id, ServerStreamingInvoker, Function{.unary_request = wrapper});
  }

  template <auto kMethod>
  static constexpr RawMethod ClientStreaming(uint32_t id) {
    constexpr StreamRequestFunction wrapper =
        [](CallContext& call, RawServerReaderWriter& reader) {
          return CallMethodImplFunction<kMethod>(
              call, static_cast<RawServerReader&>(reader));
        };
    return RawMethod(
        id, ClientStreamingInvoker, Function{.stream_request = wrapper});
  }

  template <auto kMethod>
  static constexpr RawMethod BidirectionalStreaming(uint32_t id) {
    constexpr StreamRequestFunction wrapper =
        [](CallContext& call, RawServerReaderWriter& reader_writer) {
          return CallMethodImplFunction<kMethod>(call, reader_writer);
        };
    return RawMethod(
        id, BidirectionalStreamingInvoker, Function{.stream_request = wrapper});
  }

  // Represents an invalid method. Used to reduce error message verbosity.
  static constexpr RawMethod Invalid() { return {0, InvalidInvoker, {}}; }

 private:
  // Generic versions of the user-defined functions.
  using SynchronousUnaryFunction = StatusWithSize (*)(CallContext&,
                                                      ConstByteSpan,
                                                      ByteSpan);

  using UnaryRequestFunction = void (*)(CallContext&,
                                        ConstByteSpan,
                                        RawServerWriter&);

  using StreamRequestFunction = void (*)(CallContext&, RawServerReaderWriter&);

  union Function {
    SynchronousUnaryFunction synchronous_unary;
    UnaryRequestFunction unary_request;
    StreamRequestFunction stream_request;
  };

  constexpr RawMethod(uint32_t id, Invoker invoker, Function function)
      : Method(id, invoker), function_(function) {}

  static void SynchronousUnaryInvoker(const Method& method,
                                      CallContext& call,
                                      const Packet& request);

  static void ServerStreamingInvoker(const Method& method,
                                     CallContext& call,
                                     const Packet& request);

  static void ClientStreamingInvoker(const Method& method,
                                     CallContext& call,
                                     const Packet&);

  static void BidirectionalStreamingInvoker(const Method& method,
                                            CallContext& call,
                                            const Packet&);

  // Stores the user-defined RPC.
  Function function_;
};

// Expected function signatures for user-implemented RPC functions.
using RawSynchronousUnary = StatusWithSize(ServerContext&,
                                           ConstByteSpan,
                                           ByteSpan);
using RawServerStreaming = void(ServerContext&,
                                ConstByteSpan,
                                RawServerWriter&);
using RawClientStreaming = void(ServerContext&, RawServerReader&);
using RawBidirectionalStreaming = void(ServerContext&, RawServerReaderWriter&);

// MethodTraits specialization for a static raw unary method.
template <>
struct MethodTraits<RawSynchronousUnary*> {
  using Implementation = RawMethod;

  static constexpr MethodType kType = MethodType::kUnary;
  static constexpr bool kServerStreaming = false;
  static constexpr bool kClientStreaming = false;
};

// MethodTraits specialization for a raw unary method.
template <typename T>
struct MethodTraits<RawSynchronousUnary(T::*)>
    : MethodTraits<RawSynchronousUnary*> {
  using Service = T;
};

// MethodTraits specialization for a static raw server streaming method.
template <>
struct MethodTraits<RawServerStreaming*> {
  using Implementation = RawMethod;
  static constexpr MethodType kType = MethodType::kServerStreaming;
  static constexpr bool kServerStreaming = true;
  static constexpr bool kClientStreaming = false;
};

// MethodTraits specialization for a raw server streaming method.
template <typename T>
struct MethodTraits<RawServerStreaming(T::*)>
    : MethodTraits<RawServerStreaming*> {
  using Service = T;
};

// MethodTraits specialization for a static raw client streaming method.
template <>
struct MethodTraits<RawClientStreaming*> {
  using Implementation = RawMethod;
  static constexpr MethodType kType = MethodType::kClientStreaming;
  static constexpr bool kServerStreaming = false;
  static constexpr bool kClientStreaming = true;
};

// MethodTraits specialization for a raw client streaming method.
template <typename T>
struct MethodTraits<RawClientStreaming(T::*)>
    : MethodTraits<RawClientStreaming*> {
  using Service = T;
};

// MethodTraits specialization for a static raw bidirectional streaming method.
template <>
struct MethodTraits<RawBidirectionalStreaming*> {
  using Implementation = RawMethod;
  static constexpr MethodType kType = MethodType::kBidirectionalStreaming;
  static constexpr bool kServerStreaming = true;
  static constexpr bool kClientStreaming = true;
};

// MethodTraits specialization for a raw bidirectional streaming method.
template <typename T>
struct MethodTraits<RawBidirectionalStreaming(T::*)>
    : MethodTraits<RawBidirectionalStreaming*> {
  using Service = T;
};

}  // namespace pw::rpc::internal
