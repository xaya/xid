// Copyright (C) 2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "config.h"

#include "rpc-stubs/lightserverstub.h"

#include "nonstaterpc.hpp"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <google/protobuf/stubs/common.h>

#include <curl/curl.h>

#include <json/json.h>
#include <jsonrpccpp/common/errors.h>
#include <jsonrpccpp/common/exception.h>
#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/httpserver.h>

#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <sstream>

DEFINE_int32 (game_rpc_port, 0,
              "the port at which xid's JSON-RPC server will be started");
DEFINE_string (rest_endpoint, "",
               "the endpoint of the REST API that is used to query state");

namespace xid
{
namespace
{

/**
 * Simple utility class corresponding to a "running main loop" that
 * can be stopped and waited on to be stopped.
 */
class MainLoop
{

private:

  /** Mutex for interacting with the loop.  */
  std::mutex mut;

  /** Condition variable used for waiting on the loop.  */
  std::condition_variable cv;

  /** Whether or not the loop is running.  */
  bool running = true;

public:

  /**
   * Constructs the loop instance, which will be in the "running" (i.e. not
   * yet stopped) state.
   */
  MainLoop () = default;

  MainLoop (const MainLoop&) = delete;
  void operator= (const MainLoop&) = delete;

  /**
   * Signals the loop to stop.
   */
  void
  Stop ()
  {
    std::lock_guard<std::mutex> lock(mut);
    running = false;
    cv.notify_all ();
  }

  /**
   * Waits for the loop to stop.
   */
  void
  Wait ()
  {
    std::unique_lock<std::mutex> lock(mut);
    while (running)
      cv.wait (lock);
  }

};

/**
 * Simple wrapper around the cURL easy interface to request data from
 * an HTTP GET endpoint (i.e. the REST API).
 */
class CurlRequest
{

private:

  /** The underlying cURL handle.  */
  CURL* handle;

  /** Error buffer.  */
  std::string errBuffer;

  /** Actual error message (from cURL or us).  */
  std::ostringstream error;

  /** Buffer into which the response data is saved.  */
  std::string data;

  /** The parsed data as JSON value.  */
  Json::Value jsonData;

  /**
   * Sets a given option on the cURL handle.
   */
  template <typename T>
    void
    SetOption (CURLoption option, const T& param)
  {
    CHECK_EQ (curl_easy_setopt (handle, option, param), CURLE_OK);
  }

  /**
   * cURL write callback that saves the bytes into our data string.
   */
  static size_t
  WriteCallback (const char* ptr, const size_t sz, const size_t n,
                 CurlRequest* self)
  {
    CHECK_EQ (sz, 1);
    self->data.append (ptr, n);
    return n;
  }

public:

  CurlRequest ();

  ~CurlRequest ()
  {
    curl_easy_cleanup (handle);
  }

  /**
   * Performs URL encoding of a string.
   */
  std::string
  UrlEncode (const std::string& str)
  {
    char* ptr = curl_easy_escape (handle, str.data (), str.size ());
    CHECK (ptr != nullptr);
    std::string res(ptr);
    curl_free (ptr);
    return res;
  }

  /**
   * Start a request to the given URL.  Returns true on success and false
   * if something went wrong.
   */
  bool Request (const std::string& url);

  /**
   * Returns the data for success.
   */
  const Json::Value&
  GetData () const
  {
    return jsonData;
  }

  /**
   * Returns the error message in case of an error.
   */
  std::string
  GetError () const
  {
    return error.str ();
  }

};

CurlRequest::CurlRequest ()
{
  handle = curl_easy_init ();
  CHECK (handle != nullptr);

  /* Let cURL store error messages into our error string.  */
  errBuffer.resize (CURL_ERROR_SIZE);
  SetOption (CURLOPT_ERRORBUFFER, errBuffer.data ());

  /* Enforce TLS verification.  */
  SetOption (CURLOPT_SSL_VERIFYPEER, 1L);
  SetOption (CURLOPT_SSL_VERIFYHOST, 2L);

  /* Install our write callback.  */
  SetOption (CURLOPT_WRITEFUNCTION, &WriteCallback);
  SetOption (CURLOPT_WRITEDATA, this);
}

bool
CurlRequest::Request (const std::string& url)
{
  VLOG (1) << "Requesting data from " << url << "...";

  data.clear ();
  SetOption (CURLOPT_URL, url.c_str ());

  if (curl_easy_perform (handle) != CURLE_OK)
    {
      LOG (WARNING)
          << "cURL request for " << url << " failed: " << errBuffer.c_str ();
      error << "cURL error: " << errBuffer.c_str ();
      return false;
    }

  long code;
  CHECK_EQ (curl_easy_getinfo (handle, CURLINFO_RESPONSE_CODE, &code),
            CURLE_OK);

  if (code != 200)
    {
      LOG (WARNING)
          << "cURL request for " << url << " returned status: " << code;
      error << "HTTP response status: " << code;
      return false;
    }

  VLOG (1) << "Request successful";
  VLOG (2) << "Return data:\n" << data;

  Json::CharReaderBuilder rbuilder;
  rbuilder["allowComments"] = false;
  rbuilder["strictRoot"] = true;
  rbuilder["allowDroppedNullPlaceholders"] = false;
  rbuilder["allowNumericKeys"] = false;
  rbuilder["allowSingleQuotes"] = false;
  rbuilder["failIfExtra"] = true;
  rbuilder["rejectDupKeys"] = true;
  rbuilder["allowSpecialFloats"] = false;

  std::string parseErrs;
  std::istringstream in(data);
  try
    {
      if (!Json::parseFromStream (rbuilder, in, &jsonData, &parseErrs))
        {
          LOG (WARNING)
              << "Failed to parse response data as JSON: " << parseErrs
              << "\n" << data;
          error << "JSON parser: " << parseErrs;
          return false;
        }
    }
  catch (const Json::Exception& exc)
    {
      LOG (WARNING)
          << "JSON parser threw: " << exc.what ()
          << "\n" << data;
      error << "JSON parser: " << exc.what ();
      return false;
    }

    return true;
}

/**
 * The main RPC server implementation for the light API.
 */
class LightServer : public LightServerStub
{

private:

  /** NonStateRpc instance for answering those queries.  */
  NonStateRpc nonState;

  /** Main loop that will be stopped on request.  */
  MainLoop& loop;

public:

  explicit LightServer (MainLoop& l, jsonrpc::AbstractServerConnector& conn)
    : LightServerStub(conn), loop(l)
  {}

  void stop () override;
  Json::Value getnullstate () override;
  Json::Value getnamestate (const std::string& name) override;

  Json::Value
  getauthmessage (const std::string& application,
                  const Json::Value& data,
                  const std::string& name) override
  {
    return nonState.getauthmessage (application, data, name);
  }

  std::string
  setauthsignature (const std::string& password,
                    const std::string& signature) override
  {
    return nonState.setauthsignature (password, signature);
  }

};

void
LightServer::stop ()
{
  LOG (INFO) << "RPC method called: stop";
  loop.Stop ();
}

Json::Value
LightServer::getnullstate ()
{
  LOG (INFO) << "RPC method called: getnullstate";
  /* FIXME: Implement based on REST API.  */
  return Json::Value ();
}

Json::Value
LightServer::getnamestate (const std::string& name)
{
  LOG (INFO) << "RPC method called: getnamestate " << name;

  CurlRequest req;
  std::ostringstream url;
  url << FLAGS_rest_endpoint << "/name/";
  url << req.UrlEncode (name);

  if (!req.Request (url.str ()))
    throw jsonrpc::JsonRpcException (jsonrpc::Errors::ERROR_RPC_INTERNAL_ERROR,
                                     req.GetError ());

  return req.GetData ();
}

} // anonymous namespace
} // namespace xid

int
main (int argc, char** argv)
{
  google::InitGoogleLogging (argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  CHECK_EQ (curl_global_init (CURL_GLOBAL_ALL), 0);

  gflags::SetUsageMessage ("Run Xaya ID light interface");
  gflags::SetVersionString (PACKAGE_VERSION);
  gflags::ParseCommandLineFlags (&argc, &argv, true);

  if (FLAGS_game_rpc_port == 0)
    {
      std::cerr << "Error: --game_rpc_port must be specified" << std::endl;
      return EXIT_FAILURE;
    }
  if (FLAGS_rest_endpoint.empty ())
    {
      std::cerr << "Error: --rest_endpoint must be specified" << std::endl;
      return EXIT_FAILURE;
    }

  xid::MainLoop loop;

  jsonrpc::HttpServer http(FLAGS_game_rpc_port);
  xid::LightServer srv(loop, http);

  LOG (INFO) << "Using REST API at " << FLAGS_rest_endpoint;
  LOG (INFO) << "Starting local RPC server on port " << FLAGS_game_rpc_port;
  srv.StartListening ();

  /* Note that the main loop implementation here does not catch and handle
     signals.  Thus if the process is interrupted and not shut down via RPC,
     it will simply terminate without executing the rest of main.  This is
     not an issue, though, as there is nothing to worry about in this case
     anyway.  */
  loop.Wait ();

  LOG (INFO) << "Stopping the local RPC server...";
  srv.StopListening ();

  google::protobuf::ShutdownProtobufLibrary ();
  return EXIT_SUCCESS;
}
