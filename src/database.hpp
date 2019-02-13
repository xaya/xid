// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_DATABASE_HPP
#define XID_DATABASE_HPP

#include <sqlite3.h>

#include <string>

namespace xid
{

/**
 * Simple wrapper and utility class for the database connection with
 * extra features (e.g. preparation of statements).
 */
class Database
{

protected:

  Database () = default;

public:

  virtual ~Database () = default;

  Database (const Database&) = delete;
  void operator= (const Database&) = delete;

  /**
   * Prepares a database statement from the given SQL string.  The returned
   * statement is managed and must not be freed by the caller.
   */
  virtual sqlite3_stmt* PrepareStatement (const std::string& sql) = 0;

};

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
