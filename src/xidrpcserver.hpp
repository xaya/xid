// Copyright (C) 2019-2025 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_XIDRPCSERVER_HPP
#define XID_XIDRPCSERVER_HPP

#include "rpc-stubs/xidrpcserverstub.h"

#include "logic.hpp"
#include "nonstaterpc.hpp"

#include <xayagame/game.hpp>
#include <xayagame/rpc-stubs/xayawalletrpcclient.h>

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

  /** NonStateRpc instance we use to answer those calls.  */
  NonStateRpc nonState;

  /** The underlying Game instance that manages everything.  */
  xaya::Game& game;

  /** The game logic implementation.  */
  XidGame& logic;

  /**
   * Whether or not to allow "unsafe" RPC methods (like stop, that should
   * not be publicly exposed).
   */
  bool unsafeMethods = false;

  /**
   * Checks if unsafe methods are allowed.  If not, throws a JSON-RPC
   * exception to the caller.
   */
  void EnsureUnsafeAllowed (const std::string& method) const;

public:

  explicit XidRpcServer (xaya::Game& g, XidGame& l,
                         jsonrpc::AbstractServerConnector& conn)
    : XidRpcServerStub(conn), game(g), logic(l)
  {}

  /**
   * Turns on support for unsafe methods, which should not be publicly
   * exposed.
   */
  void EnableUnsafeMethods ();

  void stop () override;
  Json::Value getcurrentstate () override;
  Json::Value getnullstate () override;
  std::string waitforchange (const std::string& knownBlock) override;

  Json::Value getnamestate (const std::string& name) override;

  Json::Value getauthmessage (const std::string& application,
                              const Json::Value& data,
                              const std::string& name) override;
  std::string setauthsignature (const std::string& password,
                                const std::string& signature) override;
  Json::Value verifyauth (const std::string& application,
                          const std::string& name,
                          const std::string& password) override;

};

} // namespace xid

#endif // XID_XIDRPCSERVER_HPP
