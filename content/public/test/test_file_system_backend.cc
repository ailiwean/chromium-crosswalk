// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/test_file_system_backend.h"

#include <set>
#include <string>
#include <vector>

#include "base/file_util.h"
#include "base/files/file.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/sequenced_task_runner.h"
#include "base/thread_task_runner_handle.h"
#include "webkit/browser/blob/file_stream_reader.h"
#include "webkit/browser/fileapi/copy_or_move_file_validator.h"
#include "webkit/browser/fileapi/file_observers.h"
#include "webkit/browser/fileapi/file_system_operation.h"
#include "webkit/browser/fileapi/file_system_operation_context.h"
#include "webkit/browser/fileapi/file_system_quota_util.h"
#include "webkit/browser/fileapi/local_file_util.h"
#include "webkit/browser/fileapi/native_file_util.h"
#include "webkit/browser/fileapi/quota/quota_reservation.h"
#include "webkit/browser/fileapi/sandbox_file_stream_writer.h"
#include "webkit/browser/fileapi/watcher_manager.h"
#include "webkit/browser/quota/quota_manager.h"
#include "webkit/common/fileapi/file_system_util.h"

using storage::FileSystemContext;
using storage::FileSystemOperation;
using storage::FileSystemOperationContext;
using storage::FileSystemURL;

namespace content {

namespace {

// Stub implementation of storage::LocalFileUtil.
class TestFileUtil : public storage::LocalFileUtil {
 public:
  explicit TestFileUtil(const base::FilePath& base_path)
      : base_path_(base_path) {}
  virtual ~TestFileUtil() {}

  // LocalFileUtil overrides.
  virtual base::File::Error GetLocalFilePath(
      FileSystemOperationContext* context,
      const FileSystemURL& file_system_url,
      base::FilePath* local_file_path) OVERRIDE {
    *local_file_path = base_path_.Append(file_system_url.path());
    return base::File::FILE_OK;
  }

 private:
  base::FilePath base_path_;
};

// Stub implementation of storage::WatcherManager. Emits a fake notification
// after a directory watcher is set successfully.
class TestWatcherManager : public storage::WatcherManager {
 public:
  TestWatcherManager() : weak_ptr_factory_(this) {}
  virtual ~TestWatcherManager() {}

  // storage::WatcherManager overrides.
  virtual void AddObserver(Observer* observer) OVERRIDE {
    observers_.AddObserver(observer);
  }

  virtual void RemoveObserver(Observer* observer) OVERRIDE {
    observers_.RemoveObserver(observer);
  }

  virtual bool HasObserver(Observer* observer) const OVERRIDE {
    return observers_.HasObserver(observer);
  }

  virtual void WatchDirectory(const storage::FileSystemURL& url,
                              bool recursive,
                              const StatusCallback& callback) OVERRIDE {
    if (recursive) {
      base::ThreadTaskRunnerHandle::Get()->PostTask(
          FROM_HERE,
          base::Bind(callback, base::File::FILE_ERROR_INVALID_OPERATION));
      return;
    }

    const GURL gurl = url.ToGURL();
    if (watched_urls_.find(gurl) != watched_urls_.end()) {
      base::ThreadTaskRunnerHandle::Get()->PostTask(
          FROM_HERE, base::Bind(callback, base::File::FILE_ERROR_EXISTS));
      return;
    }

    watched_urls_.insert(gurl);
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(callback, base::File::FILE_OK));

    // Send a fake changed notification.
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE,
        base::Bind(&TestWatcherManager::SendFakeChangeNotification,
                   weak_ptr_factory_.GetWeakPtr(),
                   url));

    // Send a fake removed notification.
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE,
        base::Bind(&TestWatcherManager::SendFakeRemoveNotification,
                   weak_ptr_factory_.GetWeakPtr(),
                   url));
  }

  virtual void UnwatchEntry(const storage::FileSystemURL& url,
                            const StatusCallback& callback) OVERRIDE {
    const GURL gurl = url.ToGURL();
    if (watched_urls_.find(gurl) == watched_urls_.end()) {
      base::ThreadTaskRunnerHandle::Get()->PostTask(
          FROM_HERE, base::Bind(callback, base::File::FILE_ERROR_NOT_FOUND));
      return;
    }

    watched_urls_.erase(gurl);
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(callback, base::File::FILE_OK));
  }

 private:
  // Sends a fake notification to each observer about a changed entry
  // represented by |url|, as long as it is still being watched.
  void SendFakeChangeNotification(const storage::FileSystemURL& url) {
    if (watched_urls_.find(url.ToGURL()) == watched_urls_.end())
      return;

    FOR_EACH_OBSERVER(Observer, observers_, OnEntryChanged(url));
  }

  // Sends a fake notification to each observer about a removed entry
  // represented by |url|, as long as it is still being watched.
  void SendFakeRemoveNotification(const storage::FileSystemURL& url) {
    if (watched_urls_.find(url.ToGURL()) == watched_urls_.end())
      return;

    FOR_EACH_OBSERVER(Observer, observers_, OnEntryRemoved(url));
  }

  ObserverList<Observer> observers_;
  std::set<GURL> watched_urls_;

  base::WeakPtrFactory<TestWatcherManager> weak_ptr_factory_;
};

}  // namespace

// This only supports single origin.
class TestFileSystemBackend::QuotaUtil : public storage::FileSystemQuotaUtil,
                                         public storage::FileUpdateObserver {
 public:
  explicit QuotaUtil(base::SequencedTaskRunner* task_runner)
      : usage_(0),
        task_runner_(task_runner) {
    update_observers_ = update_observers_.AddObserver(this, task_runner_.get());
  }
  virtual ~QuotaUtil() {}

  // FileSystemQuotaUtil overrides.
  virtual base::File::Error DeleteOriginDataOnFileTaskRunner(
      FileSystemContext* context,
      storage::QuotaManagerProxy* proxy,
      const GURL& origin_url,
      storage::FileSystemType type) OVERRIDE {
    NOTREACHED();
    return base::File::FILE_OK;
  }

  virtual scoped_refptr<storage::QuotaReservation>
  CreateQuotaReservationOnFileTaskRunner(
      const GURL& origin_url,
      storage::FileSystemType type) OVERRIDE {
    NOTREACHED();
    return scoped_refptr<storage::QuotaReservation>();
  }

  virtual void GetOriginsForTypeOnFileTaskRunner(
      storage::FileSystemType type,
      std::set<GURL>* origins) OVERRIDE {
    NOTREACHED();
  }

  virtual void GetOriginsForHostOnFileTaskRunner(
      storage::FileSystemType type,
      const std::string& host,
      std::set<GURL>* origins) OVERRIDE {
    NOTREACHED();
  }

  virtual int64 GetOriginUsageOnFileTaskRunner(
      FileSystemContext* context,
      const GURL& origin_url,
      storage::FileSystemType type) OVERRIDE {
    return usage_;
  }

  virtual void AddFileUpdateObserver(
      storage::FileSystemType type,
      FileUpdateObserver* observer,
      base::SequencedTaskRunner* task_runner) OVERRIDE {
    NOTIMPLEMENTED();
  }

  virtual void AddFileChangeObserver(
      storage::FileSystemType type,
      storage::FileChangeObserver* observer,
      base::SequencedTaskRunner* task_runner) OVERRIDE {
    change_observers_ = change_observers_.AddObserver(observer, task_runner);
  }

  virtual void AddFileAccessObserver(
      storage::FileSystemType type,
      storage::FileAccessObserver* observer,
      base::SequencedTaskRunner* task_runner) OVERRIDE {
    NOTIMPLEMENTED();
  }

  virtual const storage::UpdateObserverList* GetUpdateObservers(
      storage::FileSystemType type) const OVERRIDE {
    return &update_observers_;
  }

  virtual const storage::ChangeObserverList* GetChangeObservers(
      storage::FileSystemType type) const OVERRIDE {
    return &change_observers_;
  }

  virtual const storage::AccessObserverList* GetAccessObservers(
      storage::FileSystemType type) const OVERRIDE {
    return NULL;
  }

  // FileUpdateObserver overrides.
  virtual void OnStartUpdate(const FileSystemURL& url) OVERRIDE {}
  virtual void OnUpdate(const FileSystemURL& url, int64 delta) OVERRIDE {
    usage_ += delta;
  }
  virtual void OnEndUpdate(const FileSystemURL& url) OVERRIDE {}

  base::SequencedTaskRunner* task_runner() { return task_runner_.get(); }

 private:
  int64 usage_;

  scoped_refptr<base::SequencedTaskRunner> task_runner_;

  storage::UpdateObserverList update_observers_;
  storage::ChangeObserverList change_observers_;
};

TestFileSystemBackend::TestFileSystemBackend(
    base::SequencedTaskRunner* task_runner,
    const base::FilePath& base_path)
    : base_path_(base_path),
      file_util_(
          new storage::AsyncFileUtilAdapter(new TestFileUtil(base_path))),
      watcher_manager_(new TestWatcherManager()),
      quota_util_(new QuotaUtil(task_runner)),
      require_copy_or_move_validator_(false) {
}

TestFileSystemBackend::~TestFileSystemBackend() {
}

bool TestFileSystemBackend::CanHandleType(storage::FileSystemType type) const {
  return (type == storage::kFileSystemTypeTest);
}

void TestFileSystemBackend::Initialize(FileSystemContext* context) {
}

void TestFileSystemBackend::ResolveURL(const FileSystemURL& url,
                                       storage::OpenFileSystemMode mode,
                                       const OpenFileSystemCallback& callback) {
  callback.Run(GetFileSystemRootURI(url.origin(), url.type()),
               GetFileSystemName(url.origin(), url.type()),
               base::File::FILE_OK);
}

storage::AsyncFileUtil* TestFileSystemBackend::GetAsyncFileUtil(
    storage::FileSystemType type) {
  return file_util_.get();
}

storage::WatcherManager* TestFileSystemBackend::GetWatcherManager(
    storage::FileSystemType type) {
  return watcher_manager_.get();
}

storage::CopyOrMoveFileValidatorFactory*
TestFileSystemBackend::GetCopyOrMoveFileValidatorFactory(
    storage::FileSystemType type,
    base::File::Error* error_code) {
  DCHECK(error_code);
  *error_code = base::File::FILE_OK;
  if (require_copy_or_move_validator_) {
    if (!copy_or_move_file_validator_factory_)
      *error_code = base::File::FILE_ERROR_SECURITY;
    return copy_or_move_file_validator_factory_.get();
  }
  return NULL;
}

void TestFileSystemBackend::InitializeCopyOrMoveFileValidatorFactory(
    scoped_ptr<storage::CopyOrMoveFileValidatorFactory> factory) {
  if (!copy_or_move_file_validator_factory_)
    copy_or_move_file_validator_factory_ = factory.Pass();
}

FileSystemOperation* TestFileSystemBackend::CreateFileSystemOperation(
    const FileSystemURL& url,
    FileSystemContext* context,
    base::File::Error* error_code) const {
  scoped_ptr<FileSystemOperationContext> operation_context(
      new FileSystemOperationContext(context));
  operation_context->set_update_observers(*GetUpdateObservers(url.type()));
  operation_context->set_change_observers(
      *quota_util_->GetChangeObservers(url.type()));
  return FileSystemOperation::Create(url, context, operation_context.Pass());
}

bool TestFileSystemBackend::SupportsStreaming(
    const storage::FileSystemURL& url) const {
  return false;
}

bool TestFileSystemBackend::HasInplaceCopyImplementation(
    storage::FileSystemType type) const {
  return true;
}

scoped_ptr<storage::FileStreamReader>
TestFileSystemBackend::CreateFileStreamReader(
    const FileSystemURL& url,
    int64 offset,
    const base::Time& expected_modification_time,
    FileSystemContext* context) const {
  return scoped_ptr<storage::FileStreamReader>(
      storage::FileStreamReader::CreateForFileSystemFile(
          context, url, offset, expected_modification_time));
}

scoped_ptr<storage::FileStreamWriter>
TestFileSystemBackend::CreateFileStreamWriter(
    const FileSystemURL& url,
    int64 offset,
    FileSystemContext* context) const {
  return scoped_ptr<storage::FileStreamWriter>(
      new storage::SandboxFileStreamWriter(
          context, url, offset, *GetUpdateObservers(url.type())));
}

storage::FileSystemQuotaUtil* TestFileSystemBackend::GetQuotaUtil() {
  return quota_util_.get();
}

const storage::UpdateObserverList* TestFileSystemBackend::GetUpdateObservers(
    storage::FileSystemType type) const {
  return quota_util_->GetUpdateObservers(type);
}

void TestFileSystemBackend::AddFileChangeObserver(
    storage::FileChangeObserver* observer) {
  quota_util_->AddFileChangeObserver(
      storage::kFileSystemTypeTest, observer, quota_util_->task_runner());
}

}  // namespace content
