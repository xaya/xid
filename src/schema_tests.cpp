// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "schema.hpp"

#include "dbtest.hpp"

#include <gtest/gtest.h>

namespace xid
{
namespace
{

using SchemaTests = DBTest;

TEST_F (SchemaTests, Valid)
{
  SetupDatabaseSchema (GetDb ());
}

TEST_F (SchemaTests, TwiceIsOk)
{
  SetupDatabaseSchema (GetDb ());
  SetupDatabaseSchema (GetDb ());
}

} // anonymous namespace
} // namespace xid
