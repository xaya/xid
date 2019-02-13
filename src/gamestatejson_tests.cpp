// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gamestatejson.hpp"

#include "dbtest.hpp"
#include "testutils.hpp"

#include <gtest/gtest.h>

#include <glog/logging.h>

#include <json/json.h>

namespace xid
{

/* ************************************************************************** */

class GetNameStateTests : public DBTestWithSchema
{

protected:

  /**
   * Returns the state JSON for the given name from the test DB.
   */
  Json::Value
  GetNameState (const std::string& name)
  {
    return xid::GetNameState (GetDb (), name);
  }

};

TEST_F (GetNameStateTests, Empty)
{
  EXPECT_TRUE (JsonEquals (GetNameState ("foo"), R"({
    "signers": [] 
  })"));
}

TEST_F (GetNameStateTests, NameFiltering)
{
  Execute (R"(
    INSERT INTO `signers` (`name`, `application`, `address`)
      VALUES ("domob", NULL, "global")
  )");

  EXPECT_TRUE (JsonEquals (GetNameState ("foo"), R"({
    "signers": []
  })"));
}

TEST_F (GetNameStateTests, Signers)
{
  Execute (R"(
    INSERT INTO `signers` (`name`, `application`, `address`)
      VALUES ("domob", NULL, "global 1"),
             ("domob", NULL, "global 2"),
             ("domob", "", "empty"),
             ("domob", "app", "app 1"),
             ("domob", "app", "app 2")
  )");

  EXPECT_TRUE (JsonEquals (GetNameState ("domob"), R"({
    "signers":
      [
        {"addresses": ["global 1", "global 2"]},
        {
          "application": "",
          "addresses": ["empty"]
        },
        {
          "application": "app",
          "addresses": ["app 1", "app 2"]
        }
      ] 
  })"));
}

/* ************************************************************************** */

class GetFullStateTests : public DBTestWithSchema
{

protected:

  /**
   * Returns the full game-state JSON from the test DB.
   */
  Json::Value
  GetFullState ()
  {
    return xid::GetFullState (GetDb ());
  }

};

TEST_F (GetFullStateTests, Empty)
{
  EXPECT_TRUE (JsonEquals (GetFullState (), R"({
    "names": {}
  })"));
}

TEST_F (GetFullStateTests, WithNames)
{
  Execute (R"(
    INSERT INTO `signers` (`name`, `application`, `address`)
      VALUES ("domob", NULL, "domob 1"),
             ("domob", NULL, "domob 2"),
             ("foo", NULL, "foo")
  )");

  EXPECT_TRUE (JsonEquals (GetFullState (), R"({
    "names":
      {
        "domob":
          {
            "signers":
              [
                {"addresses": ["domob 1", "domob 2"]}
              ]
          },
        "foo":
          {
            "signers":
              [
                {"addresses": ["foo"]}
              ]
          }
      }
  })"));
}

/* ************************************************************************** */

} // namespace xid
