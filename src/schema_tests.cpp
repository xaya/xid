// Copyright (C) 2019 The Xaya developers
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
  SetupDatabaseSchema (GetHandle ());
}

TEST_F (SchemaTests, TwiceIsOk)
{
  SetupDatabaseSchema (GetHandle ());
  SetupDatabaseSchema (GetHandle ());
}

} // anonymous namespace
} // namespace xid
