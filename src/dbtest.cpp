// Copyright (C) 2019 The Xaya developers
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

/**
 * Database instance for testing with an in-memory database.
 */
class TestDatabase : public Database
{

private:

  /** The SQLite handle we use.  This is not owned by the instance.  */
  sqlite3* const handle;

  /** List of cached prepared statements.  */
  std::map<std::string, sqlite3_stmt*> stmtCache;

public:

  explicit TestDatabase (sqlite3* h)
    : handle(h)
  {}

  ~TestDatabase ()
  {
    for (const auto& stmt : stmtCache)
      sqlite3_finalize (stmt.second);
    stmtCache.clear ();
  }

  sqlite3_stmt*
  PrepareStatement (const std::string& sql) override
  {
    CHECK (handle != nullptr);

    const auto mit = stmtCache.find (sql);
    if (mit != stmtCache.end ())
      {
        sqlite3_reset (mit->second);
        CHECK_EQ (sqlite3_clear_bindings (mit->second), SQLITE_OK);
        return mit->second;
      }

    sqlite3_stmt* res = nullptr;
    CHECK_EQ (sqlite3_prepare_v2 (handle, sql.c_str (), sql.size () + 1,
                                  &res, nullptr),
              SQLITE_OK);

    stmtCache.emplace (sql, res);
    return res;
  }

};

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

  db = std::make_unique<TestDatabase> (handle);
}

DBTest::~DBTest ()
{
  LOG (INFO) << "Closing underlying SQLite database...";
  sqlite3_close (handle);
}

Database&
DBTest::GetDb ()
{
  CHECK (db != nullptr);
  return *db;
}

void
DBTest::Execute (const std::string& sql)
{
  CHECK_EQ (sqlite3_exec (handle, sql.c_str (), &ExpectNoResult,
                          nullptr, nullptr),
            SQLITE_OK);
}

DBTestWithSchema::DBTestWithSchema ()
{
  LOG (INFO) << "Setting up game-state schema in test database...";
  SetupDatabaseSchema (GetHandle ());
}

} // namespace xid
