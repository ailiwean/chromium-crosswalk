// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_QUIC_HTTP_STREAM_H_
#define NET_QUIC_QUIC_HTTP_STREAM_H_

#include <stdint.h>

#include <list>

#include "build/build_config.h"

// TODO(rtenneti): Temporary while investigating crbug.com/468529.
//                 Note base::Debug::StackTrace() is not supported in NACL
//                 builds so conditionally disabled it there.
#ifndef OS_NACL
#define TEMP_INSTRUMENTATION_468529
#endif

#ifdef TEMP_INSTRUMENTATION_468529
#include "base/debug/stack_trace.h"
#endif
#include "base/memory/weak_ptr.h"
#include "net/base/io_buffer.h"
#include "net/http/http_stream.h"
#include "net/quic/quic_chromium_client_session.h"
#include "net/quic/quic_reliable_client_stream.h"

namespace net {

namespace test {
class QuicHttpStreamPeer;
}  // namespace test

// The QuicHttpStream is a QUIC-specific HttpStream subclass.  It holds a
// non-owning pointer to a QuicReliableClientStream which it uses to
// send and receive data.
class NET_EXPORT_PRIVATE QuicHttpStream
    : public QuicChromiumClientSession::Observer,
      public QuicReliableClientStream::Delegate,
      public HttpStream {
 public:
  explicit QuicHttpStream(
      const base::WeakPtr<QuicChromiumClientSession>& session);

  ~QuicHttpStream() override;

  // HttpStream implementation.
  int InitializeStream(const HttpRequestInfo* request_info,
                       RequestPriority priority,
                       const BoundNetLog& net_log,
                       const CompletionCallback& callback) override;
  int SendRequest(const HttpRequestHeaders& request_headers,
                  HttpResponseInfo* response,
                  const CompletionCallback& callback) override;
  UploadProgress GetUploadProgress() const override;
  int ReadResponseHeaders(const CompletionCallback& callback) override;
  int ReadResponseBody(IOBuffer* buf,
                       int buf_len,
                       const CompletionCallback& callback) override;
  void Close(bool not_reusable) override;
  HttpStream* RenewStreamForAuth() override;
  bool IsResponseBodyComplete() const override;
  bool IsConnectionReused() const override;
  void SetConnectionReused() override;
  bool CanReuseConnection() const override;
  int64 GetTotalReceivedBytes() const override;
  int64_t GetTotalSentBytes() const override;
  bool GetLoadTimingInfo(LoadTimingInfo* load_timing_info) const override;
  void GetSSLInfo(SSLInfo* ssl_info) override;
  void GetSSLCertRequestInfo(SSLCertRequestInfo* cert_request_info) override;
  void Drain(HttpNetworkSession* session) override;
  void SetPriority(RequestPriority priority) override;

  // QuicReliableClientStream::Delegate implementation
  void OnHeadersAvailable(const SpdyHeaderBlock& headers) override;
  void OnDataAvailable() override;
  void OnClose(QuicErrorCode error) override;
  void OnError(int error) override;
  bool HasSendHeadersComplete() override;

  // QuicChromiumClientSession::Observer implementation
  void OnCryptoHandshakeConfirmed() override;
  void OnSessionClosed(int error) override;

 private:
  friend class test::QuicHttpStreamPeer;

#ifdef TEMP_INSTRUMENTATION_468529
  // TODO(rtenneti): Temporary while investigating crbug.com/468529
  enum Liveness {
    ALIVE = 0xCA11AB13,
    DEAD = 0xDEADBEEF,
  };
#endif

  enum State {
    STATE_NONE,
    STATE_SEND_HEADERS,
    STATE_SEND_HEADERS_COMPLETE,
    STATE_READ_REQUEST_BODY,
    STATE_READ_REQUEST_BODY_COMPLETE,
    STATE_SEND_BODY,
    STATE_SEND_BODY_COMPLETE,
    STATE_OPEN,
  };

  void OnStreamReady(int rv);
  void OnIOComplete(int rv);
  void DoCallback(int rv);

  int DoLoop(int);
  int DoSendHeaders();
  int DoSendHeadersComplete(int rv);
  int DoReadRequestBody();
  int DoReadRequestBodyComplete(int rv);
  int DoSendBody();
  int DoSendBodyComplete(int rv);
  int DoReadResponseHeaders();
  int DoReadResponseHeadersComplete(int rv);

  int ProcessResponseHeaders(const SpdyHeaderBlock& headers);

  int ReadAvailableData(IOBuffer* buf, int buf_len);

  SpdyMajorVersion GetSpdyVersion();

  void ResetStream();

  // TODO(rtenneti): Temporary while investigating crbug.com/468529
  void CrashIfInvalid() const;

  State next_state_;

  base::WeakPtr<QuicChromiumClientSession> session_;
  int session_error_;  // Error code from the connection shutdown.
  bool was_handshake_confirmed_;  // True if the crypto handshake succeeded.
  QuicChromiumClientSession::StreamRequest stream_request_;
  QuicReliableClientStream* stream_;  // Non-owning.

  // The following three fields are all owned by the caller and must
  // outlive this object, according to the HttpStream contract.

  // The request to send.
  const HttpRequestInfo* request_info_;
  // The request body to send, if any, owned by the caller.
  UploadDataStream* request_body_stream_;
  // Time the request was issued.
  base::Time request_time_;
  // The priority of the request.
  RequestPriority priority_;
  // |response_info_| is the HTTP response data object which is filled in
  // when a the response headers are read.  It is not owned by this stream.
  HttpResponseInfo* response_info_;
  // Because response data is buffered, also buffer the response status if the
  // stream is explicitly closed via OnError or OnClose with an error.
  // Once all buffered data has been returned, this will be used as the final
  // response.
  int response_status_;

  // Serialized request headers.
  SpdyHeaderBlock request_headers_;

  bool response_headers_received_;

  // Serialized HTTP request.
  std::string request_;

  // Number of bytes received when the stream was closed.
  int64 closed_stream_received_bytes_;
  // Number of bytes sent when the stream was closed.
  int64_t closed_stream_sent_bytes_;

  // The caller's callback to be used for asynchronous operations.
  CompletionCallback callback_;

  // Caller provided buffer for the ReadResponseBody() response.
  scoped_refptr<IOBuffer> user_buffer_;
  int user_buffer_len_;

  // Temporary buffer used to read the request body from UploadDataStream.
  scoped_refptr<IOBufferWithSize> raw_request_body_buf_;
  // Wraps raw_request_body_buf_ to read the remaining data progressively.
  scoped_refptr<DrainableIOBuffer> request_body_buf_;

  BoundNetLog stream_net_log_;

#ifdef TEMP_INSTRUMENTATION_468529
  // TODO(rtenneti): Temporary while investigating crbug.com/468529
  Liveness liveness_ = ALIVE;
  base::debug::StackTrace stack_trace_;
#endif

  base::WeakPtrFactory<QuicHttpStream> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(QuicHttpStream);
};

}  // namespace net

#endif  // NET_QUIC_QUIC_HTTP_STREAM_H_
