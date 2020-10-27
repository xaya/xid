// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_DBTEST_HPP
#define XID_DBTEST_HPP

#include <xayagame/sqlitestorage.hpp>

#include <gtest/gtest.h>

#include <sqlite3.h>

namespace xid
{

/**
 * Test fixture that creates and manages an in-memory SQLite database
 * to be used for testing.
 */
class DBTest : public testing::Test
{

private:

  /** The SQLiteDatabase instance.  */
  xaya::SQLiteDatabase db;

protected:

  DBTest ();

  /**
   * Returns a Database instance for the test.
   */
  xaya::SQLiteDatabase&
  GetDb ()
  {
    return db;
  }

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
