// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_DATABASE_HPP
#define XID_DATABASE_HPP

#include <sqlite3.h>

#include <string>

namespace xid
{

/**
 * Binds a typed parameter in a prepared statement.
 */
template <typename T>
  void BindParameter (sqlite3_stmt* stmt, int ind, const T& val);

/**
 * Binds a parameter to NULL.
 */
void BindParameterNull (sqlite3_stmt* stmt, int ind);

/**
 * Checks if the column with the given index is NULL.
 */
bool IsColumnNull (sqlite3_stmt* stmt, int ind);

/**
 * Extracts a typed value from the column.
 */
template <typename T>
  T GetColumnValue (sqlite3_stmt* stmt, int ind);

} // namespace xid

#endif // XID_DATABASE_HPP
