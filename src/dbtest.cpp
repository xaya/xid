// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbtest.hpp"

#include "schema.hpp"

#include <glog/logging.h>

namespace xid
{
namespace
{

/**
 * Whether or not the error handler has already been set up.  This is used
 * to ensure that we do it only once, even with many unit tests or other
 * things running in one binary.
 */
bool errorLoggerSet = false;

/**
 * Error callback for SQLite, which prints logs using glog.
 */
void
SQLiteErrorLogger (void* arg, const int errCode, const char* msg)
{
  LOG (ERROR) << "SQLite error (code " << errCode << "): " << msg;
}

} // anonymous namespace

DBTest::DBTest ()
{
  if (!errorLoggerSet)
    {
      errorLoggerSet = true;
      const int rc
          = sqlite3_config (SQLITE_CONFIG_LOG, &SQLiteErrorLogger, nullptr);
      if (rc != SQLITE_OK)
        LOG (WARNING) << "Failed to set up SQLite error handler: " << rc;
      else
        LOG (INFO) << "Configured SQLite error handler";
    }

  LOG (INFO) << "Opening in-memory SQLite database...";
  CHECK_EQ (sqlite3_open (":memory:", &handle), SQLITE_OK);
  CHECK (handle != nullptr);
}

DBTest::~DBTest ()
{
  LOG (INFO) << "Closing underlying SQLite database...";
  sqlite3_close (handle);
}

DBTestWithSchema::DBTestWithSchema ()
{
  LOG (INFO) << "Setting up game-state schema in test database...";
  SetupDatabaseSchema (GetHandle ());
}

} // namespace xid
