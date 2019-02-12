// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_SCHEMA_HPP
#define XID_SCHEMA_HPP

#include <sqlite3.h>

namespace xid
{

/**
 * Sets up the database schema (if it is not already present) on the given
 * SQLite connection.
 */
void SetupDatabaseSchema (sqlite3* db);

} // namespace xid

#endif // XID_SCHEMA_HPP
