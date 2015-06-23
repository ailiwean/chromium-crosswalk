// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_CORE_BROWSER_EXPIRE_HISTORY_BACKEND_H_
#define COMPONENTS_HISTORY_CORE_BROWSER_EXPIRE_HISTORY_BACKEND_H_

#include <queue>
#include <set>
#include <vector>

#include "base/basictypes.h"
#include "base/gtest_prod_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "components/history/core/browser/history_types.h"

class GURL;
class TestingProfile;

namespace history {

class HistoryBackendClient;
class HistoryBackendNotifier;
class HistoryDatabase;
class ThumbnailDatabase;

// Encapsulates visit expiration criteria and type of visits to expire.
class ExpiringVisitsReader {
 public:
  virtual ~ExpiringVisitsReader() {}
  // Populates |visits| from |db|, using provided |end_time| and |max_visits|
  // cap.
  virtual bool Read(base::Time end_time, HistoryDatabase* db,
                    VisitVector* visits, int max_visits) const = 0;
};

typedef std::vector<const ExpiringVisitsReader*> ExpiringVisitsReaders;

// Helper component to HistoryBackend that manages expiration and deleting of
// history.
//
// It will automatically start periodically expiring old history once you call
// StartExpiringOldStuff().
class ExpireHistoryBackend {
 public:
  // The delegate pointer must be non-null. We will NOT take ownership of it.
  // HistoryBackendClient may be null. The HistoryBackendClient is used when
  // expiring URLS so that we don't remove any URLs or favicons that are
  // bookmarked (visits are removed though).
  ExpireHistoryBackend(HistoryBackendNotifier* notifier,
                       HistoryBackendClient* backend_client,
                       scoped_refptr<base::SequencedTaskRunner> task_runner);
  ~ExpireHistoryBackend();

  // Completes initialization by setting the databases that this class will use.
  void SetDatabases(HistoryDatabase* main_db,
                    ThumbnailDatabase* thumb_db);

  // Begins periodic expiration of history older than the given threshold. This
  // will continue until the object is deleted.
  void StartExpiringOldStuff(base::TimeDelta expiration_threshold);

  // Deletes everything associated with a URL.
  void DeleteURL(const GURL& url);

  // Deletes everything associated with each URL in the list.
  void DeleteURLs(const std::vector<GURL>& url);

  // Removes all visits to restrict_urls (or all URLs if empty) in the given
  // time range, updating the URLs accordingly.
  void ExpireHistoryBetween(const std::set<GURL>& restrict_urls,
                            base::Time begin_time, base::Time end_time);

  // Removes all visits to all URLs with the given times, updating the
  // URLs accordingly.  |times| must be in reverse chronological order
  // and not contain any duplicates.
  void ExpireHistoryForTimes(const std::vector<base::Time>& times);

  // Removes the given list of visits, updating the URLs accordingly (similar to
  // ExpireHistoryBetween(), but affecting a specific set of visits).
  void ExpireVisits(const VisitVector& visits);

  // Expires all visits before and including the given time, updating the URLs
  // accordingly. Currently only used for testing.
  void ExpireHistoryBefore(base::Time end_time);

  // Returns the current cut-off time before which we will start expiring stuff.
  // Note that this as an absolute time rather than a delta, so the caller
  // should not save it.
  base::Time GetCurrentExpirationTime() const {
    return base::Time::Now() - expiration_threshold_;
  }

 private:
  FRIEND_TEST_ALL_PREFIXES(ExpireHistoryTest, DeleteFaviconsIfPossible);
  FRIEND_TEST_ALL_PREFIXES(ExpireHistoryTest, ExpireSomeOldHistory);
  FRIEND_TEST_ALL_PREFIXES(ExpireHistoryTest, ExpiringVisitsReader);
  FRIEND_TEST_ALL_PREFIXES(ExpireHistoryTest, ExpireSomeOldHistoryWithSource);
  friend class ::TestingProfile;

  struct DeleteEffects {
    DeleteEffects();
    ~DeleteEffects();

    // The time range affected. These can be is_null() to be unbounded in one
    // or both directions.
    base::Time begin_time, end_time;

    // The unique URL rows affected by this delete.
    std::map<URLID, URLRow> affected_urls;

    // The URLs modified, but not deleted, during this operation.
    URLRows modified_urls;

    // The URLs deleted during this operation.
    URLRows deleted_urls;

    // All favicon IDs that the deleted URLs had. Favicons will be shared
    // between all URLs with the same favicon, so this is the set of IDs that we
    // will need to check when the delete operations are complete.
    std::set<favicon_base::FaviconID> affected_favicons;

    // All favicon urls that were actually deleted from the thumbnail db.
    std::set<GURL> deleted_favicons;
  };

  // Deletes the visit-related stuff for all the visits in the given list, and
  // adds the rows for unique URLs affected to the affected_urls list in
  // the dependencies structure.
  void DeleteVisitRelatedInfo(const VisitVector& visits,
                              DeleteEffects* effects);

  // Finds or deletes dependency information for the given URL. Information that
  // is specific to this URL (URL row, thumbnails, etc.) is deleted.
  //
  // This does not affect the visits! This is used for expiration as well as
  // deleting from the UI, and they handle visits differently.
  //
  // Other information will be collected and returned in the output containers.
  // This includes some of the things deleted that are needed elsewhere, plus
  // some things like favicons that could be shared by many URLs, and need to
  // be checked for deletion (this allows us to delete many URLs with only one
  // check for shared information at the end).
  //
  // Assumes the main_db_ is non-NULL.
  //
  // NOTE: If the url is bookmarked, we keep the favicons and thumbnails.
  void DeleteOneURL(const URLRow& url_row,
                    bool is_bookmarked,
                    DeleteEffects* effects);

  // Deletes all favicons associated with |gurl|.
  void DeleteIcons(const GURL& gurl, DeleteEffects* effects);

  // Deletes all the URLs in the given vector and handles their dependencies.
  // This will delete starred URLs.
  void DeleteURLs(const URLRows& urls, DeleteEffects* effects);

  // Expiration involves removing visits, then propagating the visits out from
  // there and delete any orphaned URLs. These will be added to the deleted URLs
  // field of the dependencies and DeleteOneURL will handle deleting out from
  // there. This function does not handle favicons.
  //
  // When a URL is not deleted, the last visit time and the visit and typed
  // counts will be updated.
  //
  // The visits in the given vector should have already been deleted from the
  // database, and the list of affected URLs already be filled into
  // |depenencies->affected_urls|.
  //
  // Starred URLs will not be deleted. The information in the dependencies that
  // DeleteOneURL fills in will be updated, and this function will also delete
  // any now-unused favicons.
  void ExpireURLsForVisits(const VisitVector& visits, DeleteEffects* effects);

  // Deletes the favicons listed in |effects->affected_favicons| if they are
  // unsued. Fails silently (we don't care about favicons so much, so don't want
  // to stop everything if it fails). Fills |expired_favicons| with the set of
  // favicon urls that no longer have associated visits and were therefore
  // expired.
  void DeleteFaviconsIfPossible(DeleteEffects* effects);

  // Enum representing what type of action resulted in the history DB deletion.
  enum DeletionType {
    // User initiated the deletion from the History UI.
    DELETION_USER_INITIATED,
    // History data was automatically expired due to being more than 90 days
    // old.
    DELETION_EXPIRED
  };

  // Broadcasts URL modified and deleted notifications.
  void BroadcastNotifications(DeleteEffects* effects, DeletionType type);

  // Schedules a call to DoExpireIteration.
  void ScheduleExpire();

  // Calls ExpireSomeOldHistory to expire some amount of old history, according
  // to the items in work queue, and schedules another call to happen in the
  // future.
  void DoExpireIteration();

  // Tries to expire the oldest |max_visits| visits from history that are older
  // than |time_threshold|. The return value indicates if we think there might
  // be more history to expire with the current time threshold (it does not
  // indicate success or failure).
  bool ExpireSomeOldHistory(base::Time end_time,
                             const ExpiringVisitsReader* reader,
                             int max_visits);

  // Tries to detect possible bad history or inconsistencies in the database
  // and deletes items. For example, URLs with no visits.
  void ParanoidExpireHistory();

  // Initializes periodic expiration work queue by populating it with with tasks
  // for all known readers.
  void InitWorkQueue();

  // Returns the reader for all visits. This method is only used by the unit
  // tests.
  const ExpiringVisitsReader* GetAllVisitsReader();

  // Returns the reader for AUTO_SUBFRAME visits. This method is only used by
  // the unit tests.
  const ExpiringVisitsReader* GetAutoSubframeVisitsReader();

  // Non-owning pointer to the notification delegate (guaranteed non-NULL).
  HistoryBackendNotifier* notifier_;

  // Non-owning pointers to the databases we deal with (MAY BE NULL).
  HistoryDatabase* main_db_;       // Main history database.
  ThumbnailDatabase* thumb_db_;    // Thumbnails and favicons.

  // The threshold for "old" history where we will automatically delete it.
  base::TimeDelta expiration_threshold_;

  // List of all distinct types of readers. This list is used to populate the
  // work queue.
  ExpiringVisitsReaders readers_;

  // Work queue for periodic expiration tasks, used by DoExpireIteration() to
  // determine what to do at an iteration, as well as populate it for future
  // iterations.
  std::queue<const ExpiringVisitsReader*> work_queue_;

  // Readers for various types of visits.
  // TODO(dglazkov): If you are adding another one, please consider reorganizing
  // into a map.
  scoped_ptr<ExpiringVisitsReader> all_visits_reader_;
  scoped_ptr<ExpiringVisitsReader> auto_subframe_visits_reader_;

  // The HistoryBackendClient; may be null.
  HistoryBackendClient* backend_client_;

  scoped_refptr<base::SequencedTaskRunner> task_runner_;

  // Used to generate runnable methods to do timers on this class. They will be
  // automatically canceled when this class is deleted.
  base::WeakPtrFactory<ExpireHistoryBackend> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(ExpireHistoryBackend);
};

}  // namespace history

#endif  // COMPONENTS_HISTORY_CORE_BROWSER_EXPIRE_HISTORY_BACKEND_H_
