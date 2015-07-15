// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/offline_page_model.h"

#include <algorithm>

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "components/offline_pages/offline_page_item.h"
#include "components/offline_pages/offline_page_metadata_store.h"
#include "url/gurl.h"

using ArchiverResult = offline_pages::OfflinePageArchiver::ArchiverResult;
using SavePageResult = offline_pages::OfflinePageModel::Client::SavePageResult;

namespace offline_pages {

namespace {

SavePageResult ToSavePageResult(ArchiverResult archiver_result) {
  SavePageResult result;
  switch (archiver_result) {
    case ArchiverResult::SUCCESSFULLY_CREATED:
      result = SavePageResult::SUCCESS;
      break;
    case ArchiverResult::ERROR_DEVICE_FULL:
      result = SavePageResult::DEVICE_FULL;
      break;
    case ArchiverResult::ERROR_CONTENT_UNAVAILABLE:
      result = SavePageResult::CONTENT_UNAVAILABLE;
      break;
    case ArchiverResult::ERROR_ARCHIVE_CREATION_FAILED:
      result = SavePageResult::ARCHIVE_CREATION_FAILED;
      break;
    case ArchiverResult::ERROR_CANCELED:
      result = SavePageResult::CANCELLED;
      break;
    default:
      NOTREACHED();
      result = SavePageResult::CONTENT_UNAVAILABLE;
  }
  return result;
}

}  // namespace

OfflinePageModel::OfflinePageModel(
    scoped_ptr<OfflinePageMetadataStore> store,
    const scoped_refptr<base::SingleThreadTaskRunner>& task_runner)
    : store_(store.Pass()),
      task_runner_(task_runner),
      weak_ptr_factory_(this) {
}

OfflinePageModel::~OfflinePageModel() {
}

void OfflinePageModel::Shutdown() {
}

void OfflinePageModel::SavePage(
    const GURL& url,
    scoped_ptr<OfflinePageArchiver> archiver,
    const base::WeakPtr<OfflinePageModel::Client>& client) {
  DCHECK(archiver.get());
  archiver->CreateArchive(base::Bind(&OfflinePageModel::OnCreateArchiveDone,
                                     weak_ptr_factory_.GetWeakPtr(), url,
                                     client));
  pending_archivers_.push_back(archiver.Pass());
}

void OfflinePageModel::DeletePage(const GURL& url,
                                  OfflinePageModel::Client* client) {
  NOTIMPLEMENTED();
}

void OfflinePageModel::LoadAllPages(OfflinePageModel::Client* client) {
  NOTIMPLEMENTED();
}

OfflinePageMetadataStore* OfflinePageModel::GetStoreForTesting() {
  return store_.get();
}

void OfflinePageModel::OnCreateArchiveDone(
    const GURL& requested_url,
    const base::WeakPtr<OfflinePageModel::Client>& client,
    OfflinePageArchiver* archiver,
    ArchiverResult archiver_result,
    const GURL& url,
    const base::string16& title,
    const base::FilePath& file_path,
    int64 file_size) {
  if (requested_url != url) {
    DVLOG(1) << "Saved URL does not match requested URL.";
    // TODO(fgorski): We have created an archive for a wrong URL. It should be
    // deleted from here, once archiver has the right functionality.
    InformSavePageDone(client, SavePageResult::ARCHIVE_CREATION_FAILED);
    DeletePendingArchiver(archiver);
    return;
  }

  if (archiver_result != ArchiverResult::SUCCESSFULLY_CREATED) {
    SavePageResult result = ToSavePageResult(archiver_result);
    InformSavePageDone(client, result);
    DeletePendingArchiver(archiver);
    return;
  }

  OfflinePageItem offline_page_item(url, title, file_path, file_size,
                                    base::Time::Now());
  store_->AddOfflinePage(
      offline_page_item,
      base::Bind(&OfflinePageModel::OnAddOfflinePageDone,
                 weak_ptr_factory_.GetWeakPtr(), archiver, client));
}

void OfflinePageModel::OnAddOfflinePageDone(OfflinePageArchiver* archiver,
                                            const base::WeakPtr<Client>& client,
                                            bool success) {
  SavePageResult result =
      success ? SavePageResult::SUCCESS : SavePageResult::DB_FAILURE;
  InformSavePageDone(client, result);
  DeletePendingArchiver(archiver);
}

void OfflinePageModel::InformSavePageDone(const base::WeakPtr<Client>& client,
                                          SavePageResult result) {
  task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&OfflinePageModel::Client::OnSavePageDone, client, result));
}

void OfflinePageModel::DeletePendingArchiver(OfflinePageArchiver* archiver) {
  pending_archivers_.erase(std::find(
      pending_archivers_.begin(), pending_archivers_.end(), archiver));
}

}  // namespace offline_pages
