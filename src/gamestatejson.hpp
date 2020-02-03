// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_GAMESTATEJSON_HPP
#define XID_GAMESTATEJSON_HPP

#include <xayagame/sqlitestorage.hpp>

#include <json/json.h>

#include <string>

namespace xid
{

/**
 * Returns the full state of one Xaya name as JSON.  If the name is not yet
 * registered in Xid, an empty object (no signers) is returned.
 */
Json::Value GetNameState (const xaya::SQLiteDatabase& db,
                          const std::string& name);

/**
 * Returns the entire game state.  This method is not meant to be very
 * efficient.  More specific methods (e.g. GetNameState) should be preferred
 * where possible in production environments.
 */
Json::Value GetFullState (const xaya::SQLiteDatabase& db);

} // namespace xid

#endif // XID_GAMESTATEJSON_HPP
