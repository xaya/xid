// Copyright (C) 2020-2021 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "config.h"

#include "light.hpp"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <google/protobuf/stubs/common.h>

#include <cstdlib>
#include <iostream>

DEFINE_int32 (game_rpc_port, 0,
              "the port at which xid's JSON-RPC server will be started");
DEFINE_bool (game_rpc_listen_locally, true,
             "whether the game daemon's JSON-RPC server should listen locally");

DEFINE_string (rest_endpoint, "",
               "the endpoint of the REST API that is used to query state");
DEFINE_string (cafile, "",
               "if set, use this file as CA bundle instead of cURL's default");

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
  if (FLAGS_rest_endpoint.empty ())
    {
      std::cerr << "Error: --rest_endpoint must be specified" << std::endl;
      return EXIT_FAILURE;
    }

  xid::LightInstance srv(FLAGS_rest_endpoint, FLAGS_game_rpc_port);
  if (FLAGS_game_rpc_listen_locally)
    srv.EnableListenLocally ();
  srv.SetCaFile (FLAGS_cafile);

  LOG (INFO) << "Using REST API at " << FLAGS_rest_endpoint;
  LOG (INFO) << "Starting local RPC server on port " << FLAGS_game_rpc_port;
  srv.Run ();
  LOG (INFO) << "Local RPC server stopped";

  google::protobuf::ShutdownProtobufLibrary ();
  return EXIT_SUCCESS;
}
