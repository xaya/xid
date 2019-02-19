// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_XIDRPCSERVER_HPP
#define XID_XIDRPCSERVER_HPP

#include "rpc-stubs/xidrpcserverstub.h"

#include "logic.hpp"

#include <xayagame/game.hpp>

#include <json/json.h>
#include <jsonrpccpp/server.h>

#include <string>

namespace xid
{

/**
 * Implementation of the JSON-RPC interface to the game daemon.  This contains
 * some RPC calls custom to Xid.
 */
class XidRpcServer : public XidRpcServerStub
{

private:

  /** The underlying Game instance that manages everything.  */
  xaya::Game& game;

  /** The game logic implementation.  */
  XidGame& logic;

public:

  explicit XidRpcServer (xaya::Game& g, XidGame& l,
                         jsonrpc::AbstractServerConnector& conn)
    : XidRpcServerStub(conn), game(g), logic(l)
  {}

  void stop () override;
  Json::Value getcurrentstate () override;
  Json::Value waitforchange () override;

  Json::Value getnamestate (const std::string& name) override;

  Json::Value getauthmessage (const std::string& application,
                              const Json::Value& data,
                              const std::string& name) override;

};

} // namespace xid

#endif // XID_XIDRPCSERVER_HPP
