// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbtest.hpp"

#include "schema.hpp"

#include <glog/logging.h>

#include <map>
#include <string>

namespace xid
{

namespace
{

/**
 * Callback for sqlite3_exec that expects not to be called.
 */
int
ExpectNoResult (void* data, int columns, char** strs, char** names)
{
  LOG (FATAL) << "Expected no result from DB query";
}

} // anonymous namespace

DBTest::DBTest ()
  : db("test", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY)
{}

void
DBTest::Execute (const std::string& sql)
{
  CHECK_EQ (sqlite3_exec (*db, sql.c_str (), &ExpectNoResult,
                          nullptr, nullptr),
            SQLITE_OK);
}

DBTestWithSchema::DBTestWithSchema ()
{
  LOG (INFO) << "Setting up game-state schema in test database...";
  SetupDatabaseSchema (GetHandle ());
}

} // namespace xid
