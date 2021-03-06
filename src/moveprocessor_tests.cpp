// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "moveprocessor.hpp"

#include "dbtest.hpp"
#include "gamestatejson.hpp"
#include "testutils.hpp"

#include <gtest/gtest.h>

#include <glog/logging.h>

#include <json/json.h>

namespace xid
{
namespace
{

/* ************************************************************************** */

class MoveProcessorTests : public DBTestWithSchema
{

protected:

  /**
   * Runs the given string (parsed as JSON) through the move processor.
   */
  void Process (const std::string& jsonMoves)
  {
    std::istringstream in(jsonMoves);
    Json::Value val;
    in >> val;

    MoveProcessor proc(GetDb ());
    proc.ProcessAll (val);
  }

  /**
   * Expects that the state of the given name and indexed by the given
   * key (i.e. a particular subresult of it, like signers) as JSON matches
   * the given expected data (string that is parsed as JSON).
   */
  void
  ExpectNameState (const std::string& name, const std::string& key,
                   const std::string& expectedStr)
  {
    const auto actual = GetNameState (GetDb (), name);
    CHECK (actual.isMember (key));
    EXPECT_TRUE (JsonEquals (actual[key], expectedStr));
  }

};

TEST_F (MoveProcessorTests, InvalidDataFromXayaCore)
{
  for (const auto& str : {"5", "{}", "[5]", "[{}]",
                          R"([{"name": 5}])",
                          R"([{"name": "abc"}])"})
    {
      LOG (INFO) << "Testing invalid move string: " << str;
      EXPECT_DEATH (Process (str), "Check failed");
    }
}

TEST_F (MoveProcessorTests, AllMoveDataAccepted)
{
  for (const auto& mvStr : {"5", "false", "\"foo\"", "{}"})
    {
      LOG (INFO) << "Testing move data (in valid array): " << mvStr;

      std::ostringstream fullMoves;
      fullMoves
          << R"([{"name": "test", "move": )"
          << mvStr
          << "}]";

      Process (fullMoves.str ());
    }
}

/* ************************************************************************** */

class UpdateSignerTests : public MoveProcessorTests
{

protected:

  /**
   * Adds a signer entry with the given data into the test database.
   * If application is "global", then a global signer is added.
   */
  void
  AddSigner (const std::string& name, const std::string& application,
             const std::string& address)
  {
    auto stmt = GetDb ().Prepare (R"(
      INSERT INTO `signers`
        (`name`, `application`, `address`)
        VALUES (?1, ?2, ?3)
    )");

    stmt.Bind (1, name);
    if (application == "global")
      stmt.BindNull (2);
    else
      stmt.Bind (2, application);
    stmt.Bind (3, address);

    stmt.Execute ();
  }

};

TEST_F (UpdateSignerTests, BasicUpdate)
{
  AddSigner ("domob", "global", "old global 1");
  AddSigner ("domob", "global", "old global 2");
  AddSigner ("domob", "app", "old app");

  Process (R"([
    {
      "name": "domob",
      "move":
        {
          "s":
            {
              "g": ["new global 1", "new global 2"],
              "a":
                {
                  "app": ["new app"],
                  "other": ["new other"]
                }
            }
        }
    }
  ])");

  ExpectNameState ("domob", "signers", R"(
    [
      {"addresses": ["new global 1", "new global 2"]},
      {
        "application": "app",
        "addresses": ["new app"]
      },
      {
        "application": "other",
        "addresses": ["new other"]
      }
    ]
  )");
}

TEST_F (UpdateSignerTests, ClearingGlobal)
{
  AddSigner ("domob", "global", "global");
  AddSigner ("domob", "app", "app");

  Process (R"([
    {
      "name": "domob",
      "move":
        {
          "s": {"g": []}
        }
    }
  ])");

  ExpectNameState ("domob", "signers", R"(
    [
      {
        "application": "app",
        "addresses": ["app"]
      }
    ]
  )");
}

TEST_F (UpdateSignerTests, ClearingApp)
{
  AddSigner ("domob", "global", "global");
  AddSigner ("domob", "app", "app");
  AddSigner ("domob", "other", "other");

  Process (R"([
    {
      "name": "domob",
      "move":
        {
          "s": {"a": {"app": []}}
        }
    }
  ])");

  ExpectNameState ("domob", "signers", R"(
    [
      {"addresses": ["global"]},
      {
        "application": "other",
        "addresses": ["other"]
      }
    ]
  )");
}

TEST_F (UpdateSignerTests, OtherNameUntouched)
{
  AddSigner ("domob", "global", "global");
  AddSigner ("domob", "app", "app");

  Process (R"([
    {
      "name": "other",
      "move":
        {
          "s":
            {
              "g": [],
              "a": {"app": []}
            }
        }
    }
  ])");

  ExpectNameState ("domob", "signers", R"(
    [
      {"addresses": ["global"]},
      {
        "application": "app",
        "addresses": ["app"]
      }
    ]
  )");
}

TEST_F (UpdateSignerTests, EmptyApp)
{
  AddSigner ("domob", "global", "old global");
  AddSigner ("domob", "", "old app");

  Process (R"([
    {
      "name": "domob",
      "move":
        {
          "s":
            {
              "g": ["new global"],
              "a": {"": ["new app"]}
            }
        }
    }
  ])");

  ExpectNameState ("domob", "signers", R"(
    [
      {"addresses": ["new global"]},
      {
        "application": "",
        "addresses": ["new app"]
      }
    ]
  )");
}

TEST_F (UpdateSignerTests, InvalidStuffIgnored)
{
  Process (R"([
    {
      "name": "foo",
      "move": 42
    },
    {
      "name": "domob",
      "move":
        {
          "x": false,
          "s":
            {
              "g": "not an array",
              "y": -1,
              "a":
                {
                  "foo": "not an array",
                  "bar": ["addr 1", 42, "addr 2"],
                  "xyz": ["addr 3"]
                }
            }
        }
    }
  ])");

  ExpectNameState ("domob", "signers", R"(
    [
      {
        "application": "bar",
        "addresses": ["addr 1", "addr 2"]
      },
      {
        "application": "xyz",
        "addresses": ["addr 3"]
      }
    ]
  )");
}

/* ************************************************************************** */

class UpdateAddressesTests : public MoveProcessorTests
{

protected:

  /**
   * Adds an address entry with the given data into the test database.
   */
  void
  AddAddress (const std::string& name, const std::string& key,
              const std::string& address)
  {
    auto stmt = GetDb ().Prepare (R"(
      INSERT INTO `addresses`
        (`name`, `key`, `address`)
        VALUES (?1, ?2, ?3)
    )");

    stmt.Bind (1, name);
    stmt.Bind (2, key);
    stmt.Bind (3, address);

    stmt.Execute ();
  }

};

TEST_F (UpdateAddressesTests, BasicUpdate)
{
  AddAddress ("domob", "btc", "1domob");
  AddAddress ("domob", "chi", "C123456");

  Process (R"([
    {
      "name": "domob",
      "move":
        {
          "ca":
            {
              "btc": "3domob",
              "eth": "0xDomob"
            }
        }
    }
  ])");

  ExpectNameState ("domob", "addresses", R"(
    {
      "btc": "3domob",
      "chi": "C123456",
      "eth": "0xDomob"
    }
  )");
}

TEST_F (UpdateAddressesTests, Delete)
{
  AddAddress ("domob", "btc", "1domob");
  AddAddress ("domob", "chi", "C123456");

  Process (R"([
    {
      "name": "domob",
      "move":
        {
          "ca":
            {
              "chi": null,
              "ltc": null,
              "eth": "0xDomob"
            }
        }
    }
  ])");

  ExpectNameState ("domob", "addresses", R"(
    {
      "btc": "1domob",
      "eth": "0xDomob"
    }
  )");
}

TEST_F (UpdateAddressesTests, OtherNameUntouched)
{
  AddAddress ("domob", "btc", "1domob");

  Process (R"([
    {
      "name": "foo",
      "move":
        {
          "ca":
            {
              "btc": "3bar",
              "eth": "0x123456"
            }
        }
    }
  ])");

  ExpectNameState ("domob", "addresses", R"(
    {
      "btc": "1domob"
    }
  )");
}

TEST_F (UpdateAddressesTests, InvalidStuffIgnored)
{
  Process (R"([
    {
      "name": "domob",
      "move":
        {
          "x": 42,
          "ca":
            {
              "a": [],
              "y": 501,
              "z": {"invalid": "address update"},
              "btc": "1domob"
            }
        }
    }
  ])");

  ExpectNameState ("domob", "addresses", R"(
    {
      "btc": "1domob"
    }
  )");
}

/* ************************************************************************** */

} // anonymous namespace
} // namespace xid
