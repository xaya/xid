// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_MOVEPROCESSOR_HPP
#define XID_MOVEPROCESSOR_HPP

#include "database.hpp"

#include <json/json.h>

namespace xid
{

/**
 * Helper class for processing player moves and updating the game state in the
 * database accordingly.
 */
class MoveProcessor
{

private:

  /** The underlying database connection that is used for updates.  */
  Database& db;

  /**
   * Processes one entry in the moves array (given as JSON object).
   */
  void ProcessOne (const Json::Value& obj);

  /**
   * Tries to process an update to the signers in the move (if one is present).
   */
  void HandleSignerUpdate (const std::string& name, const Json::Value& mv);

public:

  explicit MoveProcessor (Database& d)
    : db(d)
  {}

  MoveProcessor (const MoveProcessor&) = delete;
  void operator= (const MoveProcessor&) = delete;

  /**
   * Processes all moves from the given JSON array.
   */
  void ProcessAll (const Json::Value& arr);

};

} // namespace xid

#endif // XID_MOVEPROCESSOR_HPP
