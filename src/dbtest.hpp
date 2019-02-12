// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_DBTEST_HPP
#define XID_DBTEST_HPP

#include "database.hpp"

#include <gtest/gtest.h>

#include <sqlite3.h>

#include <memory>

namespace xid
{

/**
 * Test fixture that creates and manages an in-memory SQLite database
 * to be used for testing.
 */
class DBTest : public testing::Test
{

private:

  /** The SQLite database handle.  */
  sqlite3* handle = nullptr;

  /** The Database instance used for testing (with statement cache).  */
  std::unique_ptr<Database> db;

protected:

  DBTest ();
  ~DBTest ();

  /**
   * Returns the underlying database handle for SQLite.
   */
  sqlite3*
  GetHandle ()
  {
    return handle;
  }

  /**
   * Returns a Database instance for the test.
   */
  Database& GetDb ();

  /**
   * Executes the given SQL statement directly.  This can be used to modify
   * the database for setting up the test (e.g. inserting data).
   */
  void Execute (const std::string& sql);

};

/**
 * Test fixture that opens an in-memory database and also installs the
 * game-state schema in it.
 */
class DBTestWithSchema : public DBTest
{

protected:

  DBTestWithSchema ();

};

} // namespace xid

#endif // XID_DBTEST_HPP
