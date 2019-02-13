// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "database.hpp"

#include <glog/logging.h>

namespace xid
{

template <>
  void
  BindParameter<std::string> (sqlite3_stmt* stmt, const int ind,
                              const std::string& val)
{
  CHECK_EQ (sqlite3_bind_text (stmt, ind, &val[0], val.size (),
                               SQLITE_TRANSIENT),
            SQLITE_OK);
}

void
BindParameterNull (sqlite3_stmt* stmt, const int ind)
{
  CHECK_EQ (sqlite3_bind_null (stmt, ind), SQLITE_OK);
}

bool
IsColumnNull (sqlite3_stmt* stmt, const int ind)
{
  return sqlite3_column_type (stmt, ind) == SQLITE_NULL;
}

template <>
  std::string
  GetColumnValue<std::string> (sqlite3_stmt* stmt, const int ind)
{
  const unsigned char* str = sqlite3_column_text (stmt, ind);
  return reinterpret_cast<const char*> (str);
}

} // namespace xid
