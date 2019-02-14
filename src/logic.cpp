// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "logic.hpp"

#include "gamestatejson.hpp"
#include "moveprocessor.hpp"
#include "schema.hpp"

#include <glog/logging.h>

namespace xid
{

/**
 * Database instance based on an SQLiteGame.
 */
class SQLiteDatabase : public Database
{

private:

  /** Underlying SQLiteGame instance.  */
  XidGame& game;

public:

  explicit SQLiteDatabase (XidGame& g)
    : game(g)
  {}

  SQLiteDatabase (const SQLiteDatabase&) = delete;
  void operator= (const SQLiteDatabase&) = delete;

  sqlite3_stmt*
  PrepareStatement (const std::string& sql) override
  {
    return game.PrepareStatement (sql);
  }

};

void
XidGame::SetupSchema (sqlite3* db)
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
      height = 585000;
      hashHex
          = "28c8c4468506f333b604c38763dd7387bd6eca2a98d1c585428b676865f9c1ec";
      break;

    case xaya::Chain::TEST:
      height = 17000;
      hashHex
          = "3bba0b9559556b202d033f69c968f2e11875d9da3c7306861358ba980eb7a84f";
      break;

    case xaya::Chain::REGTEST:
      height = 0;
      hashHex
          = "6f750b36d22f1dc3d0a6e483af45301022646dfc3b3ba2187865f5a7d6d83ab1";
      break;

    default:
      LOG (FATAL) << "Invalid chain value: " << static_cast<int> (chain);
    }
}

void
XidGame::InitialiseState (sqlite3* db)
{
  /* The initial state is simply an empty database with no defined signer
     keys or other data for any name.  */
}

void
XidGame::UpdateState (sqlite3* db, const Json::Value& blockData)
{
  SQLiteDatabase dbObj(*this);

  MoveProcessor proc(dbObj);
  proc.ProcessAll (blockData["moves"]);
}

Json::Value
XidGame::GetStateAsJson (sqlite3* db)
{
  SQLiteDatabase dbObj(*this);
  return GetFullState (dbObj);
}

Json::Value
XidGame::GetCustomStateData (xaya::Game& game, const JsonStateFromDatabase& cb)
{
  return SQLiteGame::GetCustomStateData (game, "data",
    [this, cb] (sqlite3* db)
      {
        SQLiteDatabase dbObj(*this);
        return cb (dbObj);
      });
}

} // namespace xid
