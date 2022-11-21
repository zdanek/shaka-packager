// Copyright 2016 Google LLC. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "packager/media/codecs/video_slice_header_parser.h"

#include <gtest/gtest.h>

namespace shaka {
namespace media {

TEST(H264VideoSliceHeaderParserTest, BasicSupport) {
  // Taken from bear-640x360.mp4 (video)
  const uint8_t kExtraData[] = {// Header
                                0x01, 0x64, 0x00, 0x1e, 0xff,
                                // SPS count (ignore top three bits)
                                0xe1,
                                // SPS
                                0x00, 0x19,  // Size
                                0x67, 0x64, 0x00, 0x1e, 0xac, 0xd9, 0x40, 0xa0,
                                0x2f, 0xf9, 0x70, 0x11, 0x00, 0x00, 0x03, 0x03,
                                0xe9, 0x00, 0x00, 0xea, 0x60, 0x0f, 0x16, 0x2d,
                                0x96,
                                // PPS count
                                0x01,
                                // PPS
                                0x00, 0x06,  // Size
                                0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0};
  const uint8_t kData[] = {
      // Incomplete data, but we only care about the header size.
      0x65, 0x88, 0x84, 0x00, 0x21, 0xff, 0xcf, 0x73, 0xc7, 0x24,
      0xc8, 0xc3, 0xa5, 0xcb, 0x77, 0x60, 0x50, 0x85, 0xd9, 0xfc};
  const std::vector<uint8_t> extra_data(kExtraData,
                                        kExtraData + std::size(kExtraData));

  H264VideoSliceHeaderParser parser;
  ASSERT_TRUE(parser.Initialize(extra_data));

  Nalu nalu;
  ASSERT_TRUE(nalu.Initialize(Nalu::kH264, kData, std::size(kData)));
  // Real header size is 34 bits, but we round up to 5 bytes.
  EXPECT_EQ(5, parser.GetHeaderSize(nalu));
}

TEST(H264VideoSliceHeaderParserTest, SupportsMultipleEntriesInExtraData) {
  const uint8_t kExtraData[] = {
      // Header
      0x01, 0xed, 0xf0, 0x0d, 0x00,
      // SPS count (ignore top three bits)
      0xe3,
      // SPS
      0x00, 0x19,  // Size
      0x67, 0x64, 0x00, 0x1e, 0xac, 0xd9, 0x40, 0xa0, 0x2f, 0xf9, 0x70, 0x11,
      0x00, 0x00, 0x03, 0x03, 0xe9, 0x00, 0x00, 0xea, 0x60, 0x0f, 0x16, 0x2d,
      0x96,
      // SPS
      0x00, 0x19,  // Size
      0x67, 0x64, 0x00, 0x1e, 0xac, 0xd9, 0x40, 0xa0, 0x2f, 0xf9, 0x70, 0x11,
      0x00, 0x00, 0x03, 0x03, 0xe9, 0x00, 0x00, 0xea, 0x60, 0x0f, 0x16, 0x2d,
      0x96,
      // SPS
      0x00, 0x19,  // Size
      0x67, 0x64, 0x00, 0x1e, 0xac, 0xd9, 0x40, 0xa0, 0x2f, 0xf9, 0x70, 0x11,
      0x00, 0x00, 0x03, 0x03, 0xe9, 0x00, 0x00, 0xea, 0x60, 0x0f, 0x16, 0x2d,
      0x96,
      // PPS count
      0x03,
      // PPS
      0x00, 0x06,  // Size
      0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0,
      // PPS
      0x00, 0x06,  // Size
      0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0,
      // PPS
      0x00, 0x06,  // Size
      0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0};
  const std::vector<uint8_t> extra_data(kExtraData,
                                        kExtraData + std::size(kExtraData));

  H264VideoSliceHeaderParser parser;
  EXPECT_TRUE(parser.Initialize(extra_data));
}

TEST(H264VideoSliceHeaderParserTest, IgnoresExtraDataAtEnd) {
  const uint8_t kExtraData[] = {// Header
                                0x01, 0xed, 0xf0, 0x0d, 0x00,
                                // SPS count (ignore top three bits)
                                0xe1,
                                // SPS
                                0x00, 0x19,  // Size
                                0x67, 0x64, 0x00, 0x1e, 0xac, 0xd9, 0x40, 0xa0,
                                0x2f, 0xf9, 0x70, 0x11, 0x00, 0x00, 0x03, 0x03,
                                0xe9, 0x00, 0x00, 0xea, 0x60, 0x0f, 0x16, 0x2d,
                                0x96,
                                // PPS count
                                0x00,
                                // Extra data
                                0x00, 0x19, 0x67, 0x64, 0x00};
  const std::vector<uint8_t> extra_data(kExtraData,
                                        kExtraData + std::size(kExtraData));

  H264VideoSliceHeaderParser parser;
  EXPECT_TRUE(parser.Initialize(extra_data));
}

TEST(H264VideoSliceHeaderParserTest, ErrorsForEOSAfterEntry) {
  const uint8_t kExtraData[] = {
      // Header
      0x01,
      0xed,
      0xf0,
      0x0d,
      0x00,
      // SPS count (ignore top three bits)
      0xe3,
      // SPS
      0x00,
      0x19,  // Size
      0x67,
      0x64,
      0x00,
      0x1e,
      0xac,
      0xd9,
      0x40,
      0xa0,
      0x2f,
      0xf9,
      0x70,
      0x11,
      0x00,
      0x00,
      0x03,
      0x03,
      0xe9,
      0x00,
      0x00,
      0xea,
      0x60,
      0x0f,
      0x16,
      0x2d,
      0x96,
  };
  const std::vector<uint8_t> extra_data(kExtraData,
                                        kExtraData + std::size(kExtraData));

  H264VideoSliceHeaderParser parser;
  EXPECT_FALSE(parser.Initialize(extra_data));
}

TEST(H264VideoSliceHeaderParserTest, ErrorsForEOSWithinEntry) {
  const uint8_t kExtraData[] = {
      // Header
      0x01,
      0xed,
      0xf0,
      0x0d,
      0x00,
      // SPS count (ignore top three bits)
      0xe3,
      // SPS
      0x00,
      0x19,  // Size
      0x67,
      0x64,
      0x00,
      0x1e,
      0xac,
      0xd9,
      0x40,
      0xa0,
  };
  const std::vector<uint8_t> extra_data(kExtraData,
                                        kExtraData + std::size(kExtraData));

  H264VideoSliceHeaderParser parser;
  EXPECT_FALSE(parser.Initialize(extra_data));
}

}  // namespace media
}  // namespace shaka
