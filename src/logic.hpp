// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_LOGIC_HPP
#define XID_LOGIC_HPP

#include "database.hpp"

#include <xayagame/game.hpp>
#include <xayagame/sqlitegame.hpp>

#include <sqlite3.h>

#include <json/json.h>

#include <functional>
#include <string>

namespace xid
{

class SQLiteDatabase;

/**
 * The game logic implementation for the xid game-state processor.
 */
class XidGame : public xaya::SQLiteGame
{

protected:

  void SetupSchema (sqlite3* db) override;

  void GetInitialStateBlock (unsigned& height,
                             std::string& hashHex) const override;
  void InitialiseState (sqlite3* db) override;

  void UpdateState (sqlite3* db, const Json::Value& blockData) override;

  Json::Value GetStateAsJson (sqlite3* db) override;

  friend class SQLiteDatabase;

public:

  /** Type for a callback that retrieves JSON data from the database.  */
  using JsonStateFromDatabase = std::function<Json::Value (Database& db)>;

  XidGame () = default;

  XidGame (const XidGame&) = delete;
  void operator= (const XidGame&) = delete;

  /**
   * Returns custom game-state data as JSON.  The provided callback is invoked
   * with a Database instance to retrieve the "main" state data that is returned
   * in the JSON "data" field.
   */
  Json::Value GetCustomStateData (xaya::Game& game,
                                  const JsonStateFromDatabase& cb);

};

} // namespace xid

#endif // XID_LOGIC_HPP
