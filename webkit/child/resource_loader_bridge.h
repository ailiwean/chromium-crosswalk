// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The intent of this file is to provide a type-neutral abstraction between
// Chrome and WebKit for resource loading. This pure-virtual interface is
// implemented by the embedder.
//
// One of these objects will be created by WebKit for each request. WebKit
// will own the pointer to the bridge, and will delete it when the request is
// no longer needed.
//
// In turn, the bridge's owner on the WebKit end will implement the Peer
// interface, which we will use to communicate notifications back.

#ifndef WEBKIT_CHILD_RESOURCE_LOADER_BRIDGE_H_
#define WEBKIT_CHILD_RESOURCE_LOADER_BRIDGE_H_

#include <utility>

#include "build/build_config.h"
#if defined(OS_POSIX)
#include "base/file_descriptor_posix.h"
#endif
#include "base/memory/ref_counted.h"
#include "base/platform_file.h"
#include "base/values.h"
#include "net/base/request_priority.h"
#include "url/gurl.h"
#include "webkit/child/webkit_child_export.h"

// TODO(pilgrim) remove this once resource loader is moved to content
// http://crbug.com/338338
namespace content {
class ResourceRequestBody;
struct SyncLoadResponse;
}

namespace webkit_glue {

struct ResourceResponseInfo;

class ResourceLoaderBridge {
 public:
  // Generated by the bridge. This is implemented by our custom resource loader
  // within webkit. The Peer and it's bridge should have identical lifetimes
  // as they represent each end of a communication channel.
  //
  // These callbacks mirror net::URLRequest::Delegate and the order and
  // conditions in which they will be called are identical. See url_request.h
  // for more information.
  class WEBKIT_CHILD_EXPORT Peer {
   public:
    // Called as upload progress is made.
    // note: only for requests with LOAD_ENABLE_UPLOAD_PROGRESS set
    virtual void OnUploadProgress(uint64 position, uint64 size) = 0;

    // Called when a redirect occurs.  The implementation may return false to
    // suppress the redirect.  The given ResponseInfo provides complete
    // information about the redirect, and new_url is the URL that will be
    // loaded if this method returns true.  If this method returns true, the
    // output parameter *has_new_first_party_for_cookies indicates whether the
    // output parameter *new_first_party_for_cookies contains the new URL that
    // should be consulted for the third-party cookie blocking policy.
    virtual bool OnReceivedRedirect(const GURL& new_url,
                                    const ResourceResponseInfo& info,
                                    bool* has_new_first_party_for_cookies,
                                    GURL* new_first_party_for_cookies) = 0;

    // Called when response headers are available (after all redirects have
    // been followed).
    virtual void OnReceivedResponse(const ResourceResponseInfo& info) = 0;

    // Called when a chunk of response data is downloaded.  This method may be
    // called multiple times or not at all if an error occurs.  This method is
    // only called if RequestInfo::download_to_file was set to true, and in
    // that case, OnReceivedData will not be called.
    // The encoded_data_length is the length of the encoded data transferred
    // over the network, which could be different from data length (e.g. for
    // gzipped content).
    virtual void OnDownloadedData(int len, int encoded_data_length) = 0;

    // Called when a chunk of response data is available. This method may
    // be called multiple times or not at all if an error occurs.
    // The encoded_data_length is the length of the encoded data transferred
    // over the network, which could be different from data length (e.g. for
    // gzipped content).
    virtual void OnReceivedData(const char* data,
                                int data_length,
                                int encoded_data_length) = 0;

    // Called when metadata generated by the renderer is retrieved from the
    // cache. This method may be called zero or one times.
    virtual void OnReceivedCachedMetadata(const char* data, int len) { }

    // Called when the response is complete.  This method signals completion of
    // the resource load.
    virtual void OnCompletedRequest(
        int error_code,
        bool was_ignored_by_handler,
        bool stale_copy_in_cache,
        const std::string& security_info,
        const base::TimeTicks& completion_time,
        int64 total_transfer_size) = 0;

   protected:
    virtual ~Peer() {}
  };

  // use WebKitPlatformSupportImpl::CreateResourceLoader() for construction, but
  // anybody can delete at any time, INCLUDING during processing of callbacks.
  WEBKIT_CHILD_EXPORT virtual ~ResourceLoaderBridge();

  // Call this method before calling Start() to set the request body.
  // May only be used with HTTP(S) POST requests.
  virtual void SetRequestBody(content::ResourceRequestBody* request_body) = 0;

  // Call this method to initiate the request.  If this method succeeds, then
  // the peer's methods will be called asynchronously to report various events.
  virtual bool Start(Peer* peer) = 0;

  // Call this method to cancel a request that is in progress.  This method
  // causes the request to immediately transition into the 'done' state. The
  // OnCompletedRequest method will be called asynchronously; this assumes
  // the peer is still valid.
  virtual void Cancel() = 0;

  // Call this method to suspend or resume a load that is in progress.  This
  // method may only be called after a successful call to the Start method.
  virtual void SetDefersLoading(bool value) = 0;

  // Call this method when the priority of the requested resource changes after
  // Start() has been called.  This method may only be called after a successful
  // call to the Start method.
  virtual void DidChangePriority(net::RequestPriority new_priority) = 0;

  // Call this method to load the resource synchronously (i.e., in one shot).
  // This is an alternative to the Start method.  Be warned that this method
  // will block the calling thread until the resource is fully downloaded or an
  // error occurs.  It could block the calling thread for a long time, so only
  // use this if you really need it!  There is also no way for the caller to
  // interrupt this method.  Errors are reported via the status field of the
  // response parameter.
  virtual void SyncLoad(content::SyncLoadResponse* response) = 0;

 protected:
  // Construction must go through
  // WebKitPlatformSupportImpl::CreateResourceLoader()
  // For HTTP(S) POST requests, the AppendDataToUpload and AppendFileToUpload
  // methods may be called to construct the body of the request.
  WEBKIT_CHILD_EXPORT ResourceLoaderBridge();

 private:
  DISALLOW_COPY_AND_ASSIGN(ResourceLoaderBridge);
};

}  // namespace webkit_glue

#endif  // WEBKIT_CHILD_RESOURCE_LOADER_BRIDGE_H_
