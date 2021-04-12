// Copyright (C) 2020-2021 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "light.hpp"

#include "rpc-stubs/lightserverstub.h"

#include "nonstaterpc.hpp"

#include <xayagame/rest.hpp>

#include <glog/logging.h>

#include <json/json.h>
#include <jsonrpccpp/common/errors.h>
#include <jsonrpccpp/common/exception.h>
#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/httpserver.h>

#include <condition_variable>
#include <mutex>

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
 * The main RPC server implementation for the light API.
 */
class LightServer : public LightServerStub
{

private:

  /** NonStateRpc instance for answering those queries.  */
  NonStateRpc nonState;

  /** Main loop that will be stopped on request.  */
  MainLoop& loop;

  /** REST client for requests.  */
  xaya::RestClient client;

public:

  explicit LightServer (const std::string& endpoint,
                        MainLoop& l, jsonrpc::AbstractServerConnector& conn)
    : LightServerStub(conn), loop(l), client(endpoint)
  {}

  /**
   * Sets the trusted root CA file for the TLS connection.
   */
  void
  SetCaFile (const std::string& path)
  {
    client.SetCaFile (path);
  }

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

  xaya::RestClient::Request req(client);
  if (!req.Send ("/state"))
    throw jsonrpc::JsonRpcException (jsonrpc::Errors::ERROR_RPC_INTERNAL_ERROR,
                                     req.GetError ());

  if (req.GetType () != "application/json")
    throw jsonrpc::JsonRpcException (jsonrpc::Errors::ERROR_RPC_INTERNAL_ERROR,
                                     "expected JSON response");

  return req.GetJson ();
}

Json::Value
LightServer::getnamestate (const std::string& name)
{
  LOG (INFO) << "RPC method called: getnamestate " << name;

  xaya::RestClient::Request req(client);
  if (!req.Send ("/name/" + req.UrlEncode (name)))
    throw jsonrpc::JsonRpcException (jsonrpc::Errors::ERROR_RPC_INTERNAL_ERROR,
                                     req.GetError ());

  if (req.GetType () != "application/json")
    throw jsonrpc::JsonRpcException (jsonrpc::Errors::ERROR_RPC_INTERNAL_ERROR,
                                     "expected JSON response");

  return req.GetJson ();
}

} // anonymous namespace

class LightInstance::Impl
{

private:

  /** The main loop being run.  */
  MainLoop loop;

  /** The local HTTP server for RPC requests.  */
  jsonrpc::HttpServer http;

  /** The actual xid-light RPC server.  */
  LightServer srv;

  friend class LightInstance;

public:

  explicit Impl (const std::string& endpoint, const int rpcPort)
    : http(rpcPort), srv(endpoint, loop, http)
  {}

};

LightInstance::LightInstance (const std::string& endpoint, const int rpcPort)
  : impl(new Impl (endpoint, rpcPort))
{}

LightInstance::~LightInstance () = default;

void
LightInstance::EnableListenLocally ()
{
  impl->http.BindLocalhost ();
}

void
LightInstance::SetCaFile (const std::string& path)
{
  impl->srv.SetCaFile (path);
}

void
LightInstance::Run ()
{
  impl->srv.StartListening ();
  impl->loop.Wait ();
  impl->srv.StopListening ();
}

} // namespace xid
