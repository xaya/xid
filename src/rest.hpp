// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_REST_HPP
#define XID_REST_HPP

#include <json/json.h>

#include <microhttpd.h>

#include <string>

namespace xid
{

/**
 * HTTP server providing a REST API for reading xid data.
 */
class RestApi
{

private:

  /** Exception class thrown for returning HTTP errors to the client.  */
  class HttpError;

  /** The port to listen on.  */
  const int port;

  /** The underlying MDH daemon.  */
  struct MHD_Daemon* daemon = nullptr;

  /**
   * Internal request handler function.  It should return the JSON value
   * we want to send on success, or throw an HttpError instance.
   */
  Json::Value Process (const std::string& url);

  /**
   * Request handler function for MHD.
   */
  static int RequestCallback (void* data, struct MHD_Connection* conn,
                              const char* url, const char* method,
                              const char* version,
                              const char* upload, size_t* uploadSize,
                              void** connData);

public:

  explicit RestApi (const int p)
    : port(p)
  {}

  ~RestApi ();

  RestApi () = delete;
  RestApi (const RestApi&) = delete;
  void operator= (const RestApi&) = delete;

  /**
   * Starts the REST server.  Processing of requests is done in a separate
   * thread, so this method returns immediately.
   */
  void Start ();

  /**
   * Shuts down the REST server.
   */
  void Stop ();

};

} // namespace xid

#endif // XID_REST_HPP
