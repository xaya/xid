// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xidrpcserver.hpp"

#include "gamestatejson.hpp"

#include <xayagame/uint256.hpp>

#include <glog/logging.h>

namespace xid
{

void
XidRpcServer::stop ()
{
  LOG (INFO) << "RPC method called: stop";
  game.RequestStop ();
}

Json::Value
XidRpcServer::getcurrentstate ()
{
  LOG (INFO) << "RPC method called: getcurrentstate";
  return game.GetCurrentJsonState ();
}

Json::Value
XidRpcServer::waitforchange ()
{
  LOG (INFO) << "RPC method called: waitforchange";

  xaya::uint256 block;
  game.WaitForChange (&block);

  /* If there is no best block so far, return JSON null.  */
  if (block.IsNull ())
    return Json::Value ();

  /* Otherwise, return the block hash.  */
  return block.ToHex ();
}

Json::Value
XidRpcServer::getnamestate (const std::string& name)
{
  LOG (INFO) << "RPC method called: getnamestate " << name;
  return logic.GetCustomStateData (game,
    [this, name] (Database& db)
      {
        return GetNameState (db, name);
      });
}

} // namespace xid
