// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_XIDRPCSERVER_HPP
#define XID_XIDRPCSERVER_HPP

#include "rpc-stubs/xaya-wallet-rpcclient.h"
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

  /**
   * The RPC connection to Xaya Core that supports wallet-based functions.
   * This is only set if the wallet is explicitly enabled when running xid
   * (to prevent accidental access) and will be null otherwise.
   */
  XayaWalletRpcClient* xayaWallet = nullptr;

  /**
   * Checks if the Xaya wallet is available and throws a corresponding
   * JSON-RPC error if not.
   */
  void EnsureWalletAvailable () const;

public:

  explicit XidRpcServer (xaya::Game& g, XidGame& l,
                         jsonrpc::AbstractServerConnector& conn)
    : XidRpcServerStub(conn), game(g), logic(l)
  {}

  /**
   * Enables support for RPC methods that require the Xaya wallet.
   */
  void
  EnableWallet (XayaWalletRpcClient& xw)
  {
    xayaWallet = &xw;
  }

  void stop () override;
  Json::Value getcurrentstate () override;
  Json::Value waitforchange () override;

  Json::Value getnamestate (const std::string& name) override;

  Json::Value getauthmessage (const std::string& application,
                              const Json::Value& data,
                              const std::string& name) override;
  std::string setauthsignature (const std::string& password,
                                const std::string& signature) override;
  Json::Value verifyauth (const std::string& application,
                          const std::string& name,
                          const std::string& password) override;
  Json::Value authwithwallet (const std::string& application,
                              const Json::Value& data,
                              const std::string& name) override;

};

} // namespace xid

#endif // XID_XIDRPCSERVER_HPP
