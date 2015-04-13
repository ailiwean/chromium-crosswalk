// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CHILD_SCHEDULER_WORKER_SCHEDULER_H_
#define CONTENT_CHILD_SCHEDULER_WORKER_SCHEDULER_H_

#include "base/message_loop/message_loop.h"
#include "content/child/scheduler/single_thread_idle_task_runner.h"
#include "content/common/content_export.h"

namespace base {
class MessageLoop;
}

namespace content {

class CONTENT_EXPORT WorkerScheduler {
 public:
  virtual ~WorkerScheduler();
  static scoped_ptr<WorkerScheduler> Create(base::MessageLoop* message_loop);

  // Must be called before the scheduler can be used. Does any post construction
  // initialization needed such as initializing idle period detection.
  virtual void Init() = 0;

  // Returns the default task runner.
  virtual scoped_refptr<base::SingleThreadTaskRunner> DefaultTaskRunner() = 0;

  // Returns the idle task runner. Tasks posted to this runner may be reordered
  // relative to other task types and may be starved for an arbitrarily long
  // time if no idle time is available.
  virtual scoped_refptr<SingleThreadIdleTaskRunner> IdleTaskRunner() = 0;

  // Returns true if a currently running idle task could exceed its deadline
  // without impacting user experience too much. This should only be used if
  // there is a task which cannot be pre-empted and is likely to take longer
  // than the largest expected idle task deadline. It should NOT be polled to
  // check whether more work can be performed on the current idle task after
  // its deadline has expired - post a new idle task for the continuation of the
  // work in this case.
  // Must be called from the worker's thread.
  virtual bool CanExceedIdleDeadlineIfRequired() const = 0;

  // Adds or removes a task observer from the scheduler. The observer will be
  // notified before and after every executed task. These functions can only be
  // called on the main thread.
  virtual void AddTaskObserver(
      base::MessageLoop::TaskObserver* task_observer) = 0;
  virtual void RemoveTaskObserver(
      base::MessageLoop::TaskObserver* task_observer) = 0;

  // Shuts down the scheduler by dropping any remaining pending work in the work
  // queues. After this call any work posted to the task runners will be
  // silently dropped.
  virtual void Shutdown() = 0;

 protected:
  WorkerScheduler();
  DISALLOW_COPY_AND_ASSIGN(WorkerScheduler);
};

}  // namespace content

#endif  // CONTENT_CHILD_SCHEDULER_WORKER_SCHEDULER_H_
