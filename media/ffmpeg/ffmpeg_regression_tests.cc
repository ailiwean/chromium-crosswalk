// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Regression tests for FFmpeg.  Test files can be found in the internal media
// test data directory:
//
//    https://chrome-internal.googlesource.com/chrome/data/media
//
// Simply add the custom_dep below to your gclient and sync:
//
//    "src/media/test/data/internal":
//        "https://chrome-internal.googlesource.com/chrome/data/media"
//
// Many of the files here do not cause issues outside of tooling, so you'll need
// to run this test under ASAN, TSAN, and Valgrind to ensure that all issues are
// caught.
//
// Test cases labeled FLAKY may not always pass, but they should never crash or
// cause any kind of warnings or errors under tooling.

#include <string>

#include "base/bind.h"
#include "media/test/pipeline_integration_test_base.h"

namespace media {

const char kRegressionTestDataPathPrefix[] = "internal/";

struct RegressionTestData {
  RegressionTestData(const char* filename,
                     PipelineStatus init_status,
                     PipelineStatus end_status)
      : filename(std::string(kRegressionTestDataPathPrefix) + filename),
        init_status(init_status),
        end_status(end_status) {}

  std::string filename;
  PipelineStatus init_status;
  PipelineStatus end_status;
};

// Used for tests which just need to run without crashing or tooling errors, but
// which may have undefined PipelineStatus results.
struct FlakyRegressionTestData {
  FlakyRegressionTestData(const char* filename)
      : filename(std::string(kRegressionTestDataPathPrefix) + filename) {
  }

  std::string filename;
};

class FFmpegRegressionTest
    : public testing::TestWithParam<RegressionTestData>,
      public PipelineIntegrationTestBase {
};

class FlakyFFmpegRegressionTest
    : public testing::TestWithParam<FlakyRegressionTestData>,
      public PipelineIntegrationTestBase {
};

#define FFMPEG_TEST_CASE(name, fn, init_status, end_status) \
  INSTANTIATE_TEST_CASE_P(                                  \
      name, FFmpegRegressionTest,                           \
      testing::Values(RegressionTestData(fn, init_status, end_status)));

#define FLAKY_FFMPEG_TEST_CASE(name, fn) \
    INSTANTIATE_TEST_CASE_P(FLAKY_##name, FlakyFFmpegRegressionTest, \
                            testing::Values(FlakyRegressionTestData(fn)));

// Test cases from issues.
FFMPEG_TEST_CASE(Cr47325, "security/47325.mp4", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr47761, "crbug47761.ogg", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr50045, "crbug50045.mp4", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr62127, "crbug62127.webm", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr93620, "security/93620.ogg", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr100492,
                 "security/100492.webm",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(Cr100543, "security/100543.webm", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr101458, "security/101458.webm", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr108416, "security/108416.webm", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr110849,
                 "security/110849.mkv",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_NO_SUPPORTED_STREAMS);
FFMPEG_TEST_CASE(Cr112384, "security/112384.webm", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr112976, "security/112976.ogg", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr116927,
                 "security/116927.ogv",
                 DEMUXER_ERROR_NO_SUPPORTED_STREAMS,
                 DEMUXER_ERROR_NO_SUPPORTED_STREAMS);
FFMPEG_TEST_CASE(Cr117912,
                 "security/117912.webm",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(Cr123481, "security/123481.ogv", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr132779,
                 "security/132779.webm",
                 DEMUXER_ERROR_COULD_NOT_PARSE,
                 DEMUXER_ERROR_COULD_NOT_PARSE);
FFMPEG_TEST_CASE(Cr140165, "security/140165.ogg", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr140647,
                 "security/140647.ogv",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(Cr142738, "crbug142738.ogg", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr152691,
                 "security/152691.mp3",
                 PIPELINE_OK,
                 PIPELINE_ERROR_DECODE);
FFMPEG_TEST_CASE(Cr161639, "security/161639.m4a", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr222754,
                 "security/222754.mp4",
                 PIPELINE_OK,
                 PIPELINE_ERROR_DECODE);
FFMPEG_TEST_CASE(Cr234630a, "security/234630a.mov", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr234630b,
                 "security/234630b.mov",
                 PIPELINE_OK,
                 PIPELINE_ERROR_DECODE);
FFMPEG_TEST_CASE(Cr242786, "security/242786.webm", PIPELINE_OK, PIPELINE_OK);
// Test for out-of-bounds access with slightly corrupt file (detection logic
// thinks it's a MONO file, but actually contains STEREO audio).
FFMPEG_TEST_CASE(Cr275590, "security/275590.m4a", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr444522, "security/444522.mp4", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(Cr444539,
                 "security/444539.m4a",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(Cr444546,
                 "security/444546.mp4",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(Cr449958,
                 "security/449958.webm",
                 PIPELINE_OK,
                 PIPELINE_ERROR_DECODE);

// General MP4 test cases.
FFMPEG_TEST_CASE(MP4_0,
                 "security/aac.10419.mp4",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(MP4_1,
                 "security/clockh264aac_200021889.mp4",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(MP4_2,
                 "security/clockh264aac_200701257.mp4",
                 PIPELINE_OK,
                 PIPELINE_OK);
FFMPEG_TEST_CASE(MP4_5,
                 "security/clockh264aac_3022500.mp4",
                 DEMUXER_ERROR_NO_SUPPORTED_STREAMS,
                 DEMUXER_ERROR_NO_SUPPORTED_STREAMS);
FFMPEG_TEST_CASE(MP4_6,
                 "security/clockh264aac_344289.mp4",
                 PIPELINE_OK,
                 PIPELINE_OK);
FFMPEG_TEST_CASE(MP4_7,
                 "security/clockh264mp3_187697.mp4",
                 PIPELINE_OK,
                 PIPELINE_OK);
FFMPEG_TEST_CASE(MP4_8,
                 "security/h264.705767.mp4",
                 DEMUXER_ERROR_COULD_NOT_PARSE,
                 DEMUXER_ERROR_COULD_NOT_PARSE);
FFMPEG_TEST_CASE(MP4_9,
                 "security/smclockmp4aac_1_0.mp4",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(MP4_11, "security/null1.mp4", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(MP4_16,
                 "security/looping2.mov",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(MP4_17, "security/assert2.mov", PIPELINE_OK, PIPELINE_OK);

// General OGV test cases.
FFMPEG_TEST_CASE(OGV_1,
                 "security/out.163.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_2,
                 "security/out.391.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_5,
                 "security/smclocktheora_1_0.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_7,
                 "security/smclocktheora_1_102.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_8,
                 "security/smclocktheora_1_104.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_9,
                 "security/smclocktheora_1_110.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_10,
                 "security/smclocktheora_1_179.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_11,
                 "security/smclocktheora_1_20.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_12,
                 "security/smclocktheora_1_723.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_14,
                 "security/smclocktheora_2_10405.ogv",
                 PIPELINE_OK,
                 PIPELINE_OK);
FFMPEG_TEST_CASE(OGV_15,
                 "security/smclocktheora_2_10619.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_16,
                 "security/smclocktheora_2_1075.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_17,
                 "security/vorbis.482086.ogv",
                 PIPELINE_OK,
                 PIPELINE_OK);
FFMPEG_TEST_CASE(OGV_18,
                 "security/wav.711.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_19,
                 "security/null1.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_20,
                 "security/null2.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_21,
                 "security/assert1.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_22,
                 "security/assert2.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(OGV_23,
                 "security/assert2.ogv",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);

// General WebM test cases.
FFMPEG_TEST_CASE(WEBM_0, "security/memcpy.webm", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(WEBM_1, "security/no-bug.webm", PIPELINE_OK, PIPELINE_OK);
FFMPEG_TEST_CASE(WEBM_2,
                 "security/uninitialize.webm",
                 DEMUXER_ERROR_NO_SUPPORTED_STREAMS,
                 DEMUXER_ERROR_NO_SUPPORTED_STREAMS);
FFMPEG_TEST_CASE(WEBM_4,
                 "security/out.webm.68798.1929",
                 DECODER_ERROR_NOT_SUPPORTED,
                 DECODER_ERROR_NOT_SUPPORTED);
FFMPEG_TEST_CASE(WEBM_5, "frame_size_change.webm", PIPELINE_OK, PIPELINE_OK);

// General MKV test cases.
FFMPEG_TEST_CASE(MKV_0,
                 "security/nested_tags_lang.mka.627.628",
                 PIPELINE_OK,
                 PIPELINE_ERROR_DECODE);
FFMPEG_TEST_CASE(MKV_1,
                 "security/nested_tags_lang.mka.667.628",
                 PIPELINE_OK,
                 PIPELINE_ERROR_DECODE);

// Allocate gigabytes of memory, likely can't be run on 32bit machines.
FFMPEG_TEST_CASE(BIG_MEM_1,
                 "security/bigmem1.mov",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(BIG_MEM_2,
                 "security/looping1.mov",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FFMPEG_TEST_CASE(BIG_MEM_5,
                 "security/looping5.mov",
                 DEMUXER_ERROR_COULD_NOT_OPEN,
                 DEMUXER_ERROR_COULD_NOT_OPEN);
FLAKY_FFMPEG_TEST_CASE(BIG_MEM_3, "security/looping3.mov");
FLAKY_FFMPEG_TEST_CASE(BIG_MEM_4, "security/looping4.mov");

// Flaky under threading or for other reasons.  Per rbultje, most of these will
// never be reliable since FFmpeg does not guarantee consistency in error cases.
// We only really care that these don't cause crashes or errors under tooling.
FLAKY_FFMPEG_TEST_CASE(Cr99652, "security/99652.webm");
FLAKY_FFMPEG_TEST_CASE(Cr100464, "security/100464.webm");
FLAKY_FFMPEG_TEST_CASE(Cr111342, "security/111342.ogm");
FLAKY_FFMPEG_TEST_CASE(Cr368980, "security/368980.mp4");
FLAKY_FFMPEG_TEST_CASE(OGV_0, "security/big_dims.ogv");
FLAKY_FFMPEG_TEST_CASE(OGV_3, "security/smclock_1_0.ogv");
FLAKY_FFMPEG_TEST_CASE(OGV_4, "security/smclock.ogv.1.0.ogv");
FLAKY_FFMPEG_TEST_CASE(OGV_6, "security/smclocktheora_1_10000.ogv");
FLAKY_FFMPEG_TEST_CASE(OGV_13, "security/smclocktheora_1_790.ogv");
FLAKY_FFMPEG_TEST_CASE(MP4_3, "security/clockh264aac_300413969.mp4");
FLAKY_FFMPEG_TEST_CASE(MP4_4, "security/clockh264aac_301350139.mp4");
FLAKY_FFMPEG_TEST_CASE(MP4_12, "security/assert1.mov");
FLAKY_FFMPEG_TEST_CASE(WEBM_3, "security/out.webm.139771.2965");

// Not really flaky, but can't pass the seek test.
FLAKY_FFMPEG_TEST_CASE(MP4_10, "security/null1.m4a");
FLAKY_FFMPEG_TEST_CASE(Cr112670, "security/112670.mp4");

TEST_P(FFmpegRegressionTest, BasicPlayback) {
  if (GetParam().init_status == PIPELINE_OK) {
    ASSERT_EQ(PIPELINE_OK, Start(GetParam().filename, kClockless));
    Play();
    ASSERT_EQ(GetParam().end_status, WaitUntilEndedOrError());

    // Check for ended if the pipeline is expected to finish okay.
    if (GetParam().end_status == PIPELINE_OK) {
      ASSERT_TRUE(ended_);

      // Tack a seek on the end to catch any seeking issues.
      Seek(base::TimeDelta::FromMilliseconds(0));
    }
  } else {
    // Don't bother checking the exact status as we only care that the
    // pipeline failed to start.
    EXPECT_NE(PIPELINE_OK, Start(GetParam().filename));
  }
}

TEST_P(FlakyFFmpegRegressionTest, BasicPlayback) {
  if (Start(GetParam().filename, kClockless) == PIPELINE_OK) {
    Play();
    WaitUntilEndedOrError();
  }
}

}  // namespace media
