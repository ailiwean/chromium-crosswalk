// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_POLICY_SYSTEM_LOG_UPLOADER_H_
#define CHROME_BROWSER_CHROMEOS_POLICY_SYSTEM_LOG_UPLOADER_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"
#include "chrome/browser/chromeos/policy/upload_job.h"

namespace base {
class SequencedTaskRunner;
}

namespace policy {

// Class responsible for periodically uploading system logs, it handles the
// server responses by UploadJob:Delegate callbacks.
class SystemLogUploader : public UploadJob::Delegate {
 public:
  // Structure that stores the system log files as pairs: (file name, loaded
  // from the disk binary file data).
  typedef std::vector<std::pair<std::string, std::string>> SystemLogs;

  // Refresh constants.
  static const int64 kDefaultUploadDelayMs;
  static const int64 kErrorUploadDelayMs;

  // Http header constants to upload.
  static const char* const kNameFieldTemplate;
  static const char* const kFileTypeHeaderName;
  static const char* const kFileTypeLogFile;
  static const char* const kContentTypePlainText;

  // A delegate interface used by SystemLogUploader to read the system logs
  // from the disk and create an upload job.
  class Delegate {
   public:
    typedef base::Callback<void(scoped_ptr<SystemLogs> system_logs)>
        LogUploadCallback;

    virtual ~Delegate() {}

    // Loads system logs and invokes |upload_callback|.
    virtual void LoadSystemLogs(const LogUploadCallback& upload_callback) = 0;

    // Creates a new fully configured instance of an UploadJob. This method
    // will be called exactly once per every system log upload.
    virtual scoped_ptr<UploadJob> CreateUploadJob(
        const GURL& upload_url,
        UploadJob::Delegate* delegate) = 0;
  };

  // Constructor. Callers can inject their own Delegate. A nullptr can be passed
  // for |syslog_delegate| to use the default implementation.
  explicit SystemLogUploader(
      scoped_ptr<Delegate> syslog_delegate,
      const scoped_refptr<base::SequencedTaskRunner>& task_runner);

  ~SystemLogUploader() override;

  // Returns the time of the last upload attempt, or Time(0) if no upload has
  // ever happened.
  base::Time last_upload_attempt() const { return last_upload_attempt_; }

  // UploadJob::Delegate:
  // Callbacks handle success and failure results of upload, destroy the
  // upload job.
  void OnSuccess() override;
  void OnFailure(UploadJob::ErrorCode error_code) override;

 private:
  // Starts the system log loading process.
  void StartLogUpload();

  // The callback is invoked by the Delegate if system logs have been loaded
  // from disk, uploads system logs.
  void UploadSystemLogs(scoped_ptr<SystemLogs> system_logs);

  // Helper method that figures out when the next system log upload should
  // be scheduled.
  void ScheduleNextSystemLogUpload(base::TimeDelta frequency);

  // The number of consequent retries after the failed uploads.
  int retry_count_;

  // How long to wait between system log uploads.
  base::TimeDelta upload_frequency_;

  // The time the last upload attempt was performed.
  base::Time last_upload_attempt_;

  // TaskRunner used for scheduling upload tasks.
  const scoped_refptr<base::SequencedTaskRunner> task_runner_;

  // The upload job that is re-created on every system log upload.
  scoped_ptr<UploadJob> upload_job_;

  // The Delegate is used to load system logs and create UploadJobs.
  scoped_ptr<Delegate> syslog_delegate_;

  base::ThreadChecker thread_checker_;

  // Note: This should remain the last member so it'll be destroyed and
  // invalidate the weak pointers before any other members are destroyed.
  base::WeakPtrFactory<SystemLogUploader> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(SystemLogUploader);
};

}  // namespace policy

#endif  // CHROME_BROWSER_CHROMEOS_POLICY_SYSTEM_LOG_UPLOADER_H_
