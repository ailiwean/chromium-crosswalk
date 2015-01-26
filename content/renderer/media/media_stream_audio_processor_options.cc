// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/media_stream_audio_processor_options.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "content/common/media/media_stream_options.h"
#include "content/renderer/media/media_stream_constraints_util.h"
#include "content/renderer/media/media_stream_source.h"
#include "content/renderer/media/rtc_media_constraints.h"
#include "media/audio/audio_parameters.h"
#include "third_party/webrtc/modules/audio_processing/include/audio_processing.h"
#include "third_party/webrtc/modules/audio_processing/typing_detection.h"

namespace content {

const char MediaAudioConstraints::kEchoCancellation[] = "echoCancellation";
const char MediaAudioConstraints::kGoogEchoCancellation[] =
    "googEchoCancellation";
const char MediaAudioConstraints::kGoogExperimentalEchoCancellation[] =
    "googEchoCancellation2";
const char MediaAudioConstraints::kGoogAutoGainControl[] =
    "googAutoGainControl";
const char MediaAudioConstraints::kGoogExperimentalAutoGainControl[] =
    "googAutoGainControl2";
const char MediaAudioConstraints::kGoogNoiseSuppression[] =
    "googNoiseSuppression";
const char MediaAudioConstraints::kGoogExperimentalNoiseSuppression[] =
    "googNoiseSuppression2";
const char MediaAudioConstraints::kGoogBeamforming[] = "googBeamforming";
const char MediaAudioConstraints::kGoogHighpassFilter[] = "googHighpassFilter";
const char MediaAudioConstraints::kGoogTypingNoiseDetection[] =
    "googTypingNoiseDetection";
const char MediaAudioConstraints::kGoogAudioMirroring[] = "googAudioMirroring";

namespace {

// Constant constraint keys which enables default audio constraints on
// mediastreams with audio.
struct {
  const char* key;
  bool value;
} const kDefaultAudioConstraints[] = {
  { MediaAudioConstraints::kEchoCancellation, true },
  { MediaAudioConstraints::kGoogEchoCancellation, true },
#if defined(OS_ANDROID) || defined(OS_IOS)
  { MediaAudioConstraints::kGoogExperimentalEchoCancellation, false },
#else
  // Enable the extended filter mode AEC on all non-mobile platforms.
  { MediaAudioConstraints::kGoogExperimentalEchoCancellation, true },
#endif
  { MediaAudioConstraints::kGoogAutoGainControl, true },
  { MediaAudioConstraints::kGoogExperimentalAutoGainControl, true },
  { MediaAudioConstraints::kGoogNoiseSuppression, true },
  { MediaAudioConstraints::kGoogHighpassFilter, true },
  { MediaAudioConstraints::kGoogTypingNoiseDetection, true },
  { MediaAudioConstraints::kGoogExperimentalNoiseSuppression, false },
  { MediaAudioConstraints::kGoogBeamforming, false },
#if defined(OS_WIN)
  { kMediaStreamAudioDucking, true },
#else
  { kMediaStreamAudioDucking, false },
#endif
  { kMediaStreamAudioHotword, false },
};

bool IsAudioProcessingConstraint(const std::string& key) {
  // |kMediaStreamAudioDucking| does not require audio processing.
  return key != kMediaStreamAudioDucking;
}

// Used to log echo quality based on delay estimates.
enum DelayBasedEchoQuality {
  DELAY_BASED_ECHO_QUALITY_GOOD = 0,
  DELAY_BASED_ECHO_QUALITY_SPURIOUS,
  DELAY_BASED_ECHO_QUALITY_BAD,
  DELAY_BASED_ECHO_QUALITY_MAX
};

DelayBasedEchoQuality EchoDelayFrequencyToQuality(float delay_frequency) {
  const float kEchoDelayFrequencyLowerLimit = 0.1f;
  const float kEchoDelayFrequencyUpperLimit = 0.8f;
  // DELAY_BASED_ECHO_QUALITY_GOOD
  //   delay is out of bounds during at most 10 % of the time.
  // DELAY_BASED_ECHO_QUALITY_SPURIOUS
  //   delay is out of bounds 10-80 % of the time.
  // DELAY_BASED_ECHO_QUALITY_BAD
  //   delay is mostly out of bounds >= 80 % of the time.
  if (delay_frequency <= kEchoDelayFrequencyLowerLimit)
    return DELAY_BASED_ECHO_QUALITY_GOOD;
  else if (delay_frequency < kEchoDelayFrequencyUpperLimit)
    return DELAY_BASED_ECHO_QUALITY_SPURIOUS;
  else
    return DELAY_BASED_ECHO_QUALITY_BAD;
}

}  // namespace

// TODO(xians): Remove this method after the APM in WebRtc is deprecated.
void MediaAudioConstraints::ApplyFixedAudioConstraints(
    RTCMediaConstraints* constraints) {
  for (size_t i = 0; i < arraysize(kDefaultAudioConstraints); ++i) {
    bool already_set_value;
    if (!webrtc::FindConstraint(constraints, kDefaultAudioConstraints[i].key,
                                &already_set_value, NULL)) {
      const std::string value = kDefaultAudioConstraints[i].value ?
          webrtc::MediaConstraintsInterface::kValueTrue :
          webrtc::MediaConstraintsInterface::kValueFalse;
      constraints->AddOptional(kDefaultAudioConstraints[i].key, value, false);
    } else {
      DVLOG(1) << "Constraint " << kDefaultAudioConstraints[i].key
               << " already set to " << already_set_value;
    }
  }
}

MediaAudioConstraints::MediaAudioConstraints(
    const blink::WebMediaConstraints& constraints, int effects)
    : constraints_(constraints),
      effects_(effects),
      default_audio_processing_constraint_value_(true) {
  // The default audio processing constraints are turned off when
  // - gUM has a specific kMediaStreamSource, which is used by tab capture
  //   and screen capture.
  // - |kEchoCancellation| is explicitly set to false.
  std::string value_str;
  bool value_bool = false;
  if ((GetConstraintValueAsString(constraints, kMediaStreamSource,
                                  &value_str)) ||
      (GetConstraintValueAsBoolean(constraints_, kEchoCancellation,
                                   &value_bool) && !value_bool)) {
    default_audio_processing_constraint_value_ = false;
  }
}

MediaAudioConstraints::~MediaAudioConstraints() {}

bool MediaAudioConstraints::GetProperty(const std::string& key) {
  // Return the value if the constraint is specified in |constraints|,
  // otherwise return the default value.
  bool value = false;
  if (!GetConstraintValueAsBoolean(constraints_, key, &value))
    value = GetDefaultValueForConstraint(constraints_, key);

  return value;
}

bool MediaAudioConstraints::GetEchoCancellationProperty() {
  // If platform echo canceller is enabled, disable the software AEC.
  if (effects_ & media::AudioParameters::ECHO_CANCELLER)
    return false;

  // If |kEchoCancellation| is specified in the constraints, it will
  // override the value of |kGoogEchoCancellation|.
  bool value = false;
  if (GetConstraintValueAsBoolean(constraints_, kEchoCancellation, &value))
    return value;

  return GetProperty(kGoogEchoCancellation);
}

bool MediaAudioConstraints::IsValid() {
  blink::WebVector<blink::WebMediaConstraint> mandatory;
  constraints_.getMandatoryConstraints(mandatory);
  for (size_t i = 0; i < mandatory.size(); ++i) {
    const std::string key = mandatory[i].m_name.utf8();
    if (key == kMediaStreamSource || key == kMediaStreamSourceId ||
        key == MediaStreamSource::kSourceId) {
      // Ignore Chrome specific Tab capture and |kSourceId| constraints.
      continue;
    }

    bool valid = false;
    for (size_t j = 0; j < arraysize(kDefaultAudioConstraints); ++j) {
      if (key == kDefaultAudioConstraints[j].key) {
        bool value = false;
        valid = GetMandatoryConstraintValueAsBoolean(constraints_, key, &value);
        break;
      }
    }

    if (!valid) {
      DLOG(ERROR) << "Invalid MediaStream constraint. Name: " << key;
      return false;
    }
  }

  return true;
}

bool MediaAudioConstraints::GetDefaultValueForConstraint(
    const blink::WebMediaConstraints& constraints, const std::string& key) {
  // |kMediaStreamAudioDucking| is not restricted by
  // |default_audio_processing_constraint_value_| since it does not require
  // audio processing.
  if (!default_audio_processing_constraint_value_ &&
      IsAudioProcessingConstraint(key))
    return false;

  for (size_t i = 0; i < arraysize(kDefaultAudioConstraints); ++i) {
    if (kDefaultAudioConstraints[i].key == key)
      return kDefaultAudioConstraints[i].value;
  }

  return false;
}

EchoInformation::EchoInformation()
    : echo_poor_delay_counts_(0),
      echo_total_delay_counts_(0),
      last_log_time_(base::TimeTicks::Now()) {}

EchoInformation::~EchoInformation() {}

void EchoInformation::UpdateAecDelayStats(int delay) {
  // One way to get an indication of how well the echo cancellation performs is
  // to compare the, by AEC, estimated delay with the AEC filter length.
  // |kMaxAecFilterLengthMs| is the maximum delay we can allow before we
  // consider the AEC to fail. This value should not be larger than the filter
  // length used inside AEC. This is for now set to match the extended filter
  // mode which is turned on for all platforms.
  const int kMaxAecFilterLengthMs = 128;
  if ((delay < -2) || (delay > kMaxAecFilterLengthMs)) {
    // The |delay| is out of bounds which indicates that the echo cancellation
    // filter can not handle the echo. Hence, we have a potential full echo
    // case. |delay| values {-1, -2} are reserved for errors.
    ++echo_poor_delay_counts_;
  }
  ++echo_total_delay_counts_;
  LogAecDelayStats();
}

void EchoInformation::LogAecDelayStats() {
  // We update the UMA statistics every 5 seconds.
  const int kTimeBetweenLogsInSeconds = 5;
  const base::TimeDelta time_since_last_log =
      base::TimeTicks::Now() - last_log_time_;
  if (time_since_last_log.InSeconds() < kTimeBetweenLogsInSeconds)
    return;

  // Calculate how frequent the AEC delay was out of bounds since last time we
  // updated UMA histograms. Then store the result into one of three histogram
  // buckets; see DelayBasedEchoQuality.
  float poor_delay_frequency = 0.f;
  if (echo_total_delay_counts_ > 0) {
    poor_delay_frequency = static_cast<float>(echo_poor_delay_counts_) /
        static_cast<float>(echo_total_delay_counts_);
    UMA_HISTOGRAM_ENUMERATION("WebRTC.AecDelayBasedQuality",
                              EchoDelayFrequencyToQuality(poor_delay_frequency),
                              DELAY_BASED_ECHO_QUALITY_MAX);
  }
  echo_poor_delay_counts_ = 0;
  echo_total_delay_counts_ = 0;
  last_log_time_ = base::TimeTicks::Now();
}

void EnableEchoCancellation(AudioProcessing* audio_processing) {
#if defined(OS_ANDROID) || defined(OS_IOS)
  const std::string group_name =
      base::FieldTrialList::FindFullName("ReplaceAECMWithAEC");
  if (group_name.empty() || group_name != "Enabled") {
    // Mobile devices are using AECM.
    int err = audio_processing->echo_control_mobile()->set_routing_mode(
        webrtc::EchoControlMobile::kSpeakerphone);
    err |= audio_processing->echo_control_mobile()->Enable(true);
    CHECK_EQ(err, 0);
    return;
  }
#endif
  int err = audio_processing->echo_cancellation()->set_suppression_level(
      webrtc::EchoCancellation::kHighSuppression);

  // Enable the metrics for AEC.
  err |= audio_processing->echo_cancellation()->enable_metrics(true);
  err |= audio_processing->echo_cancellation()->enable_delay_logging(true);
  err |= audio_processing->echo_cancellation()->Enable(true);
  CHECK_EQ(err, 0);
}

void EnableNoiseSuppression(AudioProcessing* audio_processing) {
  int err = audio_processing->noise_suppression()->set_level(
      webrtc::NoiseSuppression::kHigh);
  err |= audio_processing->noise_suppression()->Enable(true);
  CHECK_EQ(err, 0);
}

void EnableHighPassFilter(AudioProcessing* audio_processing) {
  CHECK_EQ(audio_processing->high_pass_filter()->Enable(true), 0);
}

void EnableTypingDetection(AudioProcessing* audio_processing,
                           webrtc::TypingDetection* typing_detector) {
  int err = audio_processing->voice_detection()->Enable(true);
  err |= audio_processing->voice_detection()->set_likelihood(
      webrtc::VoiceDetection::kVeryLowLikelihood);
  CHECK_EQ(err, 0);

  // Configure the update period to 1s (100 * 10ms) in the typing detector.
  typing_detector->SetParameters(0, 0, 0, 0, 0, 100);
}

void StartEchoCancellationDump(AudioProcessing* audio_processing,
                               base::File aec_dump_file) {
  DCHECK(aec_dump_file.IsValid());

  FILE* stream = base::FileToFILE(aec_dump_file.Pass(), "w");
  if (!stream) {
    LOG(ERROR) << "Failed to open AEC dump file";
    return;
  }

  if (audio_processing->StartDebugRecording(stream))
    DLOG(ERROR) << "Fail to start AEC debug recording";
}

void StopEchoCancellationDump(AudioProcessing* audio_processing) {
  if (audio_processing->StopDebugRecording())
    DLOG(ERROR) << "Fail to stop AEC debug recording";
}

void EnableAutomaticGainControl(AudioProcessing* audio_processing) {
#if defined(OS_ANDROID) || defined(OS_IOS)
  const webrtc::GainControl::Mode mode = webrtc::GainControl::kFixedDigital;
#else
  const webrtc::GainControl::Mode mode = webrtc::GainControl::kAdaptiveAnalog;
#endif
  int err = audio_processing->gain_control()->set_mode(mode);
  err |= audio_processing->gain_control()->Enable(true);
  CHECK_EQ(err, 0);
}

void GetAecStats(AudioProcessing* audio_processing,
                 webrtc::AudioProcessorInterface::AudioProcessorStats* stats) {
  // These values can take on valid negative values, so use the lowest possible
  // level as default rather than -1.
  stats->echo_return_loss = -100;
  stats->echo_return_loss_enhancement = -100;

  // These values can also be negative, but in practice -1 is only used to
  // signal insufficient data, since the resolution is limited to multiples
  // of 4ms.
  stats->echo_delay_median_ms = -1;
  stats->echo_delay_std_ms = -1;

  // TODO(ajm): Re-enable this metric once we have a reliable implementation.
  stats->aec_quality_min = -1.0f;

  if (!audio_processing->echo_cancellation()->are_metrics_enabled() ||
      !audio_processing->echo_cancellation()->is_delay_logging_enabled() ||
      !audio_processing->echo_cancellation()->is_enabled()) {
    return;
  }

  // TODO(ajm): we may want to use VoECallReport::GetEchoMetricsSummary
  // here, but it appears to be unsuitable currently. Revisit after this is
  // investigated: http://b/issue?id=5666755
  webrtc::EchoCancellation::Metrics echo_metrics;
  if (!audio_processing->echo_cancellation()->GetMetrics(&echo_metrics)) {
    stats->echo_return_loss = echo_metrics.echo_return_loss.instant;
    stats->echo_return_loss_enhancement =
        echo_metrics.echo_return_loss_enhancement.instant;
  }

  int median = 0, std = 0;
  if (!audio_processing->echo_cancellation()->GetDelayMetrics(&median, &std)) {
    stats->echo_delay_median_ms = median;
    stats->echo_delay_std_ms = std;
  }
}

}  // namespace content
