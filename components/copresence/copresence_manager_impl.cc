// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/copresence/copresence_manager_impl.h"

#include <vector>

#include "base/bind.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "components/copresence/copresence_state_impl.h"
#include "components/copresence/handlers/directive_handler_impl.h"
#include "components/copresence/handlers/gcm_handler_impl.h"
#include "components/copresence/proto/rpcs.pb.h"
#include "components/copresence/public/whispernet_client.h"
#include "components/copresence/rpc/rpc_handler.h"

namespace {

const int kPollTimerIntervalMs = 3000;   // milliseconds.
const int kAudioCheckIntervalMs = 1000;  // milliseconds.

}  // namespace

namespace copresence {

// Public functions.

CopresenceManagerImpl::CopresenceManagerImpl(CopresenceDelegate* delegate)
    : delegate_(delegate),
      whispernet_init_callback_(base::Bind(
          &CopresenceManagerImpl::WhispernetInitComplete,
          // This callback gets cancelled when we are destroyed.
          base::Unretained(this))),
      init_failed_(false),
      state_(new CopresenceStateImpl),
      directive_handler_(new DirectiveHandlerImpl(
          // The directive handler and its descendants
          // will be destructed before the CopresenceState instance.
          base::Bind(&CopresenceStateImpl::UpdateDirectives,
                     base::Unretained(state_.get())))),
      poll_timer_(new base::RepeatingTimer<CopresenceManagerImpl>),
      audio_check_timer_(new base::RepeatingTimer<CopresenceManagerImpl>) {
  DCHECK(delegate_);
  DCHECK(delegate_->GetWhispernetClient());
  delegate_->GetWhispernetClient()->Initialize(
      whispernet_init_callback_.callback());

  if (delegate->GetGCMDriver())
    gcm_handler_.reset(new GCMHandlerImpl(delegate->GetGCMDriver(),
                                          directive_handler_.get()));

  rpc_handler_.reset(new RpcHandler(delegate,
                                    state_.get(),
                                    directive_handler_.get(),
                                    gcm_handler_.get()));
}

CopresenceManagerImpl::~CopresenceManagerImpl() {
  whispernet_init_callback_.Cancel();
}

CopresenceState* CopresenceManagerImpl::state() {
  return state_.get();
}

// Returns false if any operations were malformed.
void CopresenceManagerImpl::ExecuteReportRequest(
    const ReportRequest& request,
    const std::string& app_id,
    const std::string& auth_token,
    const StatusCallback& callback) {
  // If initialization has failed, reject all requests.
  if (init_failed_) {
    callback.Run(FAIL);
    return;
  }

  // We'll need to modify the ReportRequest, so we make our own copy to send.
  scoped_ptr<ReportRequest> request_copy(new ReportRequest(request));
  rpc_handler_->SendReportRequest(
      request_copy.Pass(), app_id, auth_token, callback);
}


// Private functions.

void CopresenceManagerImpl::WhispernetInitComplete(bool success) {
  if (success) {
    DVLOG(3) << "Whispernet initialized successfully.";

    directive_handler_->Start(delegate_->GetWhispernetClient(),
                              base::Bind(&CopresenceManagerImpl::ReceivedTokens,
                                         base::Unretained(this)));

    // Start up timers.
    poll_timer_->Start(FROM_HERE,
                       base::TimeDelta::FromMilliseconds(kPollTimerIntervalMs),
                       base::Bind(&CopresenceManagerImpl::PollForMessages,
                                  base::Unretained(this)));
    audio_check_timer_->Start(
        FROM_HERE, base::TimeDelta::FromMilliseconds(kAudioCheckIntervalMs),
        base::Bind(&CopresenceManagerImpl::AudioCheck, base::Unretained(this)));
  } else {
    LOG(ERROR) << "Whispernet initialization failed!";
    init_failed_ = true;
  }
}

void CopresenceManagerImpl::ReceivedTokens(
    const std::vector<AudioToken>& tokens) {
  rpc_handler_->ReportTokens(tokens);

  // Update the CopresenceState.
  for (const AudioToken audio_token : tokens) {
    DVLOG(3) << "Heard token: " << audio_token.token;
    ReceivedToken token(
        audio_token.token,
        audio_token.audible ? AUDIO_AUDIBLE_DTMF : AUDIO_ULTRASOUND_PASSBAND,
        base::Time::Now());
    state_->UpdateReceivedToken(token);
  }
}

void CopresenceManagerImpl::PollForMessages() {
  // Report our currently playing tokens.
  const std::string& audible_token =
      directive_handler_->GetCurrentAudioToken(AUDIBLE);
  const std::string& inaudible_token =
      directive_handler_->GetCurrentAudioToken(INAUDIBLE);

  std::vector<AudioToken> tokens;
  if (!audible_token.empty())
    tokens.push_back(AudioToken(audible_token, true));
  if (!inaudible_token.empty())
    tokens.push_back(AudioToken(inaudible_token, false));

  if (!tokens.empty())
    rpc_handler_->ReportTokens(tokens);
}

void CopresenceManagerImpl::AudioCheck() {
  if (!directive_handler_->GetCurrentAudioToken(AUDIBLE).empty() &&
      !directive_handler_->IsAudioTokenHeard(AUDIBLE)) {
    delegate_->HandleStatusUpdate(AUDIO_FAIL);
  } else if (!directive_handler_->GetCurrentAudioToken(INAUDIBLE).empty() &&
             !directive_handler_->IsAudioTokenHeard(INAUDIBLE)) {
    delegate_->HandleStatusUpdate(AUDIO_FAIL);
  }
}

}  // namespace copresence
