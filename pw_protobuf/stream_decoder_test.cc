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

#include "pw_protobuf/stream_decoder.h"

#include "gtest/gtest.h"
#include "pw_stream/memory_stream.h"

namespace pw::protobuf {
namespace {

TEST(StreamDecoder, Decode) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // type=int32, k=1, v=42
    0x08, 0x2a,
    // type=sint32, k=2, v=-13
    0x10, 0x19,
    // type=bool, k=3, v=false
    0x18, 0x00,
    // type=double, k=4, v=3.14159
    0x21, 0x6e, 0x86, 0x1b, 0xf0, 0xf9, 0x21, 0x09, 0x40,
    // type=fixed32, k=5, v=0xdeadbeef
    0x2d, 0xef, 0xbe, 0xad, 0xde,
    // type=string, k=6, v="Hello world"
    0x32, 0x0b, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd',
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(decoder.FieldNumber().value(), 1u);
  Result<int32_t> int32 = decoder.ReadInt32();
  ASSERT_EQ(int32.status(), OkStatus());
  EXPECT_EQ(int32.value(), 42);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(decoder.FieldNumber().value(), 2u);
  Result<int32_t> sint32 = decoder.ReadSint32();
  ASSERT_EQ(sint32.status(), OkStatus());
  EXPECT_EQ(sint32.value(), -13);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(decoder.FieldNumber().value(), 3u);
  Result<bool> boolean = decoder.ReadBool();
  ASSERT_EQ(boolean.status(), OkStatus());
  EXPECT_FALSE(boolean.value());

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(decoder.FieldNumber().value(), 4u);
  Result<double> dbl = decoder.ReadDouble();
  ASSERT_EQ(dbl.status(), OkStatus());
  EXPECT_EQ(dbl.value(), 3.14159);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(decoder.FieldNumber().value(), 5u);
  Result<uint32_t> fixed32 = decoder.ReadFixed32();
  ASSERT_EQ(fixed32.status(), OkStatus());
  EXPECT_EQ(fixed32.value(), 0xdeadbeef);

  char buffer[16];
  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(decoder.FieldNumber().value(), 6u);
  StatusWithSize sws = decoder.ReadString(buffer);
  ASSERT_EQ(sws.status(), OkStatus());
  buffer[sws.size()] = '\0';
  EXPECT_STREQ(buffer, "Hello world");

  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_SkipsUnusedFields) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // type=int32, k=1, v=42
    0x08, 0x2a,
    // type=sint32, k=2, v=-13
    0x10, 0x19,
    // type=bool, k=3, v=false
    0x18, 0x00,
    // type=double, k=4, v=3.14159
    0x21, 0x6e, 0x86, 0x1b, 0xf0, 0xf9, 0x21, 0x09, 0x40,
    // type=fixed32, k=5, v=0xdeadbeef
    0x2d, 0xef, 0xbe, 0xad, 0xde,
    // type=string, k=6, v="Hello world"
    0x32, 0x0b, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd',
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  // Don't process any fields except for the fourth. Next should still iterate
  // correctly despite field values not being consumed.
  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 4u);
  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_BadData) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // type=int32, k=1, v=42
    0x08, 0x2a,
    // type=sint32, k=2, value... missing
    0x10,
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 1u);
  Result<int32_t> int32 = decoder.ReadInt32();
  ASSERT_EQ(int32.status(), OkStatus());
  EXPECT_EQ(int32.value(), 42);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 2u);
  EXPECT_EQ(decoder.ReadSint32().status(), Status::DataLoss());

  EXPECT_EQ(decoder.Next(), Status::DataLoss());
}

TEST(Decoder, Decode_SkipsBadFieldNumbers) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // type=int32, k=1, v=42
    0x08, 0x2a,
    // type=int32, k=19001, v=42 (invalid field number)
    0xc8, 0xa3, 0x09, 0x2a,
    // type=bool, k=3, v=false
    0x18, 0x00,
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(*decoder.FieldNumber(), 1u);
  Result<int32_t> int32 = decoder.ReadInt32();
  ASSERT_EQ(int32.status(), OkStatus());
  EXPECT_EQ(int32.value(), 42);

  // Bad field.
  EXPECT_EQ(decoder.Next(), Status::DataLoss());
  EXPECT_EQ(decoder.FieldNumber().status(), Status::FailedPrecondition());

  EXPECT_EQ(decoder.Next(), Status::DataLoss());
}

TEST(StreamDecoder, Decode_Nested) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // type=int32, k=1, v=42
    0x08, 0x2a,

    // Submessage (bytes) key=8, length=4
    0x32, 0x04,
    // type=uint32, k=1, v=2
    0x08, 0x02,
    // type=uint32, k=2, v=7
    0x10, 0x07,
    // End submessage

    // type=sint32, k=2, v=-13
    0x10, 0x19,
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 1u);
  Result<int32_t> int32 = decoder.ReadInt32();
  ASSERT_EQ(int32.status(), OkStatus());
  EXPECT_EQ(int32.value(), 42);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 6u);
  {
    StreamDecoder nested = decoder.GetNestedDecoder();

    EXPECT_EQ(nested.Next(), OkStatus());
    ASSERT_EQ(*nested.FieldNumber(), 1u);
    Result<uint32_t> uint32 = nested.ReadUint32();
    ASSERT_EQ(uint32.status(), OkStatus());
    EXPECT_EQ(uint32.value(), 2u);

    EXPECT_EQ(nested.Next(), OkStatus());
    ASSERT_EQ(*nested.FieldNumber(), 2u);
    uint32 = nested.ReadUint32();
    ASSERT_EQ(uint32.status(), OkStatus());
    EXPECT_EQ(uint32.value(), 7u);

    ASSERT_EQ(nested.Next(), Status::OutOfRange());
  }

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 2u);
  Result<int32_t> sint32 = decoder.ReadSint32();
  ASSERT_EQ(sint32.status(), OkStatus());
  EXPECT_EQ(sint32.value(), -13);

  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_Nested_SeeksToNextFieldOnDestruction) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // type=int32, k=1, v=42
    0x08, 0x2a,

    // Submessage (bytes) key=8, length=4
    0x32, 0x04,
    // type=uint32, k=1, v=2
    0x08, 0x02,
    // type=uint32, k=2, v=7
    0x10, 0x07,
    // End submessage

    // type=sint32, k=2, v=-13
    0x10, 0x19,
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 1u);

  // Create a nested encoder for the nested field, but don't use it.
  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 6u);
  { StreamDecoder nested = decoder.GetNestedDecoder(); }

  // The root decoder should still advance to the next field after the nested
  // decoder is closed.
  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 2u);

  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_Nested_LastField) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // type=int32, k=1, v=42
    0x08, 0x2a,

    // Submessage (bytes) key=8, length=4
    0x32, 0x04,
    // type=uint32, k=1, v=2
    0x08, 0x02,
    // type=uint32, k=2, v=7
    0x10, 0x07,
    // End submessage and proto
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 1u);

  // Create a nested encoder for the nested field, which is the last field in
  // the root proto.
  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 6u);
  { StreamDecoder nested = decoder.GetNestedDecoder(); }

  // Root decoder should correctly terminate after the nested decoder is closed.
  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_Nested_MultiLevel) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // Submessage key=1, length=4
    0x0a, 0x04,

    // Sub-submessage key=1, length=2
    0x0a, 0x02,
    // type=uint32, k=2, v=7
    0x10, 0x07,
    // End sub-submessage

    // End submessage
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 1u);
  {
    StreamDecoder nested = decoder.GetNestedDecoder();

    EXPECT_EQ(nested.Next(), OkStatus());
    ASSERT_EQ(*nested.FieldNumber(), 1u);

    {
      StreamDecoder double_nested = nested.GetNestedDecoder();

      EXPECT_EQ(double_nested.Next(), OkStatus());
      ASSERT_EQ(*double_nested.FieldNumber(), 2u);
      Result<uint32_t> result = double_nested.ReadUint32();
      ASSERT_EQ(result.status(), OkStatus());
      EXPECT_EQ(result.value(), 7u);

      EXPECT_EQ(double_nested.Next(), Status::OutOfRange());
    }

    EXPECT_EQ(nested.Next(), Status::OutOfRange());
  }

  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_Nested_InvalidField) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // Submessage key=1, length=4
    0x0a, 0x04,

    // Oops. No data!
  };

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 1u);
  {
    StreamDecoder nested = decoder.GetNestedDecoder();
    EXPECT_EQ(nested.Next(), Status::DataLoss());
  }

  EXPECT_EQ(decoder.Next(), Status::DataLoss());
}

TEST(StreamDecoder, Decode_BytesReader) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // bytes key=1, length=14
    0x0a, 0x0e,

    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d,
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(*decoder.FieldNumber(), 1u);
  {
    StreamDecoder::BytesReader bytes = decoder.GetBytesReader();
    EXPECT_EQ(bytes.field_size(), 14u);

    std::byte buffer[7];
    EXPECT_EQ(bytes.Read(buffer).status(), OkStatus());
    EXPECT_EQ(std::memcmp(buffer, encoded_proto + 2, sizeof(buffer)), 0);

    EXPECT_EQ(bytes.Read(buffer).status(), OkStatus());
    EXPECT_EQ(std::memcmp(buffer, encoded_proto + 9, sizeof(buffer)), 0);

    EXPECT_EQ(bytes.Read(buffer).status(), Status::OutOfRange());
  }

  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_BytesReader_Seek) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // bytes key=1, length=14
    0x0a, 0x0e,

    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d,
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(*decoder.FieldNumber(), 1u);
  {
    StreamDecoder::BytesReader bytes = decoder.GetBytesReader();

    std::byte buffer[2];

    ASSERT_EQ(bytes.Seek(3), OkStatus());

    EXPECT_EQ(bytes.Read(buffer).status(), OkStatus());
    EXPECT_EQ(std::memcmp(buffer, encoded_proto + 5, sizeof(buffer)), 0);

    // Bad seek offset (absolute).
    ASSERT_EQ(bytes.Seek(15), Status::OutOfRange());

    // Seek back from current position.
    ASSERT_EQ(bytes.Seek(-4, stream::Stream::kCurrent), OkStatus());

    EXPECT_EQ(bytes.Read(buffer).status(), OkStatus());
    EXPECT_EQ(std::memcmp(buffer, encoded_proto + 3, sizeof(buffer)), 0);

    // Bad seek offset (relative).
    ASSERT_EQ(bytes.Seek(-4, stream::Stream::kCurrent), Status::OutOfRange());

    // Seek from the end of the bytes field.
    ASSERT_EQ(bytes.Seek(-2, stream::Stream::kEnd), OkStatus());

    EXPECT_EQ(bytes.Read(buffer).status(), OkStatus());
    EXPECT_EQ(std::memcmp(buffer, encoded_proto + 14, sizeof(buffer)), 0);

    // Bad seek offset (end).
    ASSERT_EQ(bytes.Seek(-15, stream::Stream::kEnd), Status::OutOfRange());
  }

  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_BytesReader_Close) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // bytes key=1, length=14
    0x0a, 0x0e,

    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d,
    // End bytes

    // type=sint32, k=2, v=-13
    0x10, 0x19,
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(*decoder.FieldNumber(), 1u);
  {
    // Partially consume the bytes field.
    StreamDecoder::BytesReader bytes = decoder.GetBytesReader();

    std::byte buffer[2];
    EXPECT_EQ(bytes.Read(buffer).status(), OkStatus());
    EXPECT_EQ(std::memcmp(buffer, encoded_proto + 2, sizeof(buffer)), 0);
  }

  // Continue reading the top-level message.
  EXPECT_EQ(decoder.Next(), OkStatus());
  EXPECT_EQ(*decoder.FieldNumber(), 2u);

  EXPECT_EQ(decoder.Next(), Status::OutOfRange());
}

TEST(StreamDecoder, Decode_BytesReader_InvalidField) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // bytes key=1, length=4
    0x0a, 0x04,

    // Oops. No data!
  };

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  EXPECT_EQ(decoder.Next(), OkStatus());
  ASSERT_EQ(*decoder.FieldNumber(), 1u);
  {
    StreamDecoder::BytesReader bytes = decoder.GetBytesReader();
    EXPECT_EQ(bytes.Seek(0), Status::DataLoss());

    std::byte buffer[2];
    EXPECT_EQ(bytes.Read(buffer).status(), Status::DataLoss());
  }

  EXPECT_EQ(decoder.Next(), Status::DataLoss());
}

TEST(StreamDecoder, GetLengthDelimitedPayloadBounds) {
  // clang-format off
  constexpr uint8_t encoded_proto[] = {
    // bytes key=1, length=14
    0x0a, 0x0e,

    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d,
    // End bytes

    // type=sint32, k=2, v=-13
    0x10, 0x19,
  };
  // clang-format on

  stream::MemoryReader reader(std::as_bytes(std::span(encoded_proto)));
  StreamDecoder decoder(reader);

  ASSERT_EQ(OkStatus(), decoder.Next());
  Result<StreamDecoder::Bounds> field_bound =
      decoder.GetLengthDelimitedPayloadBounds();
  ASSERT_EQ(OkStatus(), field_bound.status());
  ASSERT_EQ(field_bound.value().low, 2ULL);
  ASSERT_EQ(field_bound.value().high, 16ULL);

  ASSERT_EQ(OkStatus(), decoder.Next());
  ASSERT_EQ(Status::NotFound(),
            decoder.GetLengthDelimitedPayloadBounds().status());
}

}  // namespace
}  // namespace pw::protobuf
