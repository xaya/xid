// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_LOGIC_HPP
#define XID_LOGIC_HPP

#include <xayagame/game.hpp>
#include <xayagame/sqlitegame.hpp>
#include <xayagame/sqlitestorage.hpp>

#include <sqlite3.h>

#include <json/json.h>

#include <functional>
#include <mutex>
#include <string>

namespace xid
{

/**
 * The game logic implementation for the xid game-state processor.
 */
class XidGame : public xaya::SQLiteGame
{

private:

  /** Mutex protecting access to the shared XayaRpcClient.  */
  std::mutex rpcMut;

protected:

  void SetupSchema (xaya::SQLiteDatabase& db) override;

  void GetInitialStateBlock (unsigned& height,
                             std::string& hashHex) const override;
  void InitialiseState (xaya::SQLiteDatabase& db) override;

  void UpdateState (xaya::SQLiteDatabase& db,
                    const Json::Value& blockData) override;

  Json::Value GetStateAsJson (const xaya::SQLiteDatabase& db) override;

public:

  /** Type for a callback that retrieves JSON data from the database.  */
  using JsonStateFromDatabase
      = std::function<Json::Value (const xaya::SQLiteDatabase& db)>;

  XidGame () = default;

  XidGame (const XidGame&) = delete;
  void operator= (const XidGame&) = delete;

  /**
   * Exposes xaya::VerifyMessage with the configured RPC connection.  This is
   * used by the verifyauth RPC call.
   */
  std::string VerifyMessage (const std::string& msg, const std::string& sgn);

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
