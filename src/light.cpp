// Copyright (C) 2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "config.h"

#include "rpc-stubs/lightserverstub.h"

#include "nonstaterpc.hpp"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <google/protobuf/stubs/common.h>

#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/httpserver.h>

#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>

DEFINE_int32 (game_rpc_port, 0,
              "the port at which xid's JSON-RPC server will be started");

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
  /* FIXME: Implement using the REST API.  */
  return Json::Value ();
}

} // anonymous namespace
} // namespace xid

int
main (int argc, char** argv)
{
  google::InitGoogleLogging (argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  gflags::SetUsageMessage ("Run Xaya ID light interface");
  gflags::SetVersionString (PACKAGE_VERSION);
  gflags::ParseCommandLineFlags (&argc, &argv, true);

  if (FLAGS_game_rpc_port == 0)
    {
      std::cerr << "Error: --game_rpc_port must be specified" << std::endl;
      return EXIT_FAILURE;
    }

  xid::MainLoop loop;

  jsonrpc::HttpServer http(FLAGS_game_rpc_port);
  xid::LightServer srv(loop, http);

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
