// Copyright (C) 2019-2022 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "logic.hpp"

#include "gamestatejson.hpp"
#include "moveprocessor.hpp"
#include "schema.hpp"

#include <xayagame/signatures.hpp>

#include <glog/logging.h>

namespace xid
{

void
XidGame::SetupSchema (xaya::SQLiteDatabase& db)
{
  SetupDatabaseSchema (db);
}

void
XidGame::GetInitialStateBlock (unsigned& height, std::string& hashHex) const
{
  const xaya::Chain chain = GetChain ();
  switch (chain)
    {
    case xaya::Chain::MAIN:
      height = 585'000;
      hashHex
          = "28c8c4468506f333b604c38763dd7387bd6eca2a98d1c585428b676865f9c1ec";
      break;

    case xaya::Chain::TEST:
      height = 17'000;
      hashHex
          = "3bba0b9559556b202d033f69c968f2e11875d9da3c7306861358ba980eb7a84f";
      break;

    case xaya::Chain::REGTEST:
      height = 0;
      hashHex
          = "6f750b36d22f1dc3d0a6e483af45301022646dfc3b3ba2187865f5a7d6d83ab1";
      break;

    case xaya::Chain::POLYGON:
      height = 29'260'000;
      hashHex
          = "eaa86806adf2b23b62bbb58c3f61004e2242ab2c5903384241375bfe1fb91921";
      break;

    case xaya::Chain::MUMBAI:
      height = 26'640'000;
      hashHex
          = "1e8ade976623345fd3f474cba53627b413f9155394bb7f1fb20a8270297b5e75";
      break;

    case xaya::Chain::GANACHE:
      height = 0;
      /* Ganache does not have a fixed genesis block.  So leave the block
         hash open and just accept any at height 0.  */
      hashHex = "";
      break;

    default:
      LOG (FATAL) << "Invalid chain value: " << static_cast<int> (chain);
    }
}

void
XidGame::InitialiseState (xaya::SQLiteDatabase& db)
{
  /* The initial state is simply an empty database with no defined signer
     keys or other data for any name.  */
}

void
XidGame::UpdateState (xaya::SQLiteDatabase& db, const Json::Value& blockData)
{
  MoveProcessor proc(db);
  proc.ProcessAll (blockData["moves"]);
}

Json::Value
XidGame::GetStateAsJson (const xaya::SQLiteDatabase& db)
{
  return GetFullState (db);
}

std::string
XidGame::VerifyMessage (const std::string& msg, const std::string& sgn)
{
  std::lock_guard<std::mutex> lock(rpcMut);
  return xaya::VerifyMessage (GetXayaRpc (), msg, sgn);
}

Json::Value
XidGame::GetCustomStateData (xaya::Game& game, const JsonStateFromDatabase& cb)
{
  return SQLiteGame::GetCustomStateData (game, "data",
    [this, cb] (const xaya::SQLiteDatabase& db)
      {
        return cb (db);
      });
}

} // namespace xid
