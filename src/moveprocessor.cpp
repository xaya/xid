// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "moveprocessor.hpp"

#include <glog/logging.h>

namespace xid
{

namespace
{

/**
 * Sets the list of signers for a particular application (or global signers
 * if nullptr is passed) to the given JSON array.
 */
void
SetSignerList (Database& db, const std::string& name,
               const std::string* application,
               const Json::Value& signerArr)
{
  if (application == nullptr)
    VLOG (1) << "Setting global signers for " << name << " to: " << signerArr;
  else
    VLOG (1)
        << "Setting signers for " << name
        << " and application " << *application << " to: " << signerArr;

  CHECK (signerArr.isArray ());

  sqlite3_stmt* stmt = nullptr;
  if (application == nullptr)
    stmt = db.PrepareStatement (R"(
      DELETE FROM `signers`
        WHERE `name` = ?1 AND `application` IS NULL
    )");
  else
    {
      stmt = db.PrepareStatement (R"(
        DELETE FROM `signers`
          WHERE `name` = ?1 AND `application` = ?2
      )");
      BindParameter (stmt, 2, *application);
    }
  BindParameter (stmt, 1, name);
  CHECK_EQ (sqlite3_step (stmt), SQLITE_DONE);

  stmt = db.PrepareStatement (R"(
    INSERT INTO `signers`
      (`name`, `application`, `address`)
      VALUES (?1, ?2, ?3)
  )");
  BindParameter (stmt, 1, name);
  if (application == nullptr)
    BindParameterNull (stmt, 2);
  else
    BindParameter (stmt, 2, *application);

  for (const auto& addr : signerArr)
    {
      if (!addr.isString ())
        {
          LOG (WARNING)
              << "Signer value in update for " << name
              << " is not a string: " << addr;
          continue;
        }

      BindParameter (stmt, 3, addr.asString ());
      CHECK_EQ (sqlite3_step (stmt), SQLITE_DONE);

      /* We reuse the prepared statement for all signer inserts, since they
         are just the same operation repeated.  We can even keep the bindings
         for name and application, and just override address in the next
         iteration of the loop.  */
      sqlite3_reset (stmt);
    }
}

} // anonymous namespace

void
MoveProcessor::HandleSignerUpdate (const std::string& name,
                                   const Json::Value& obj)
{
  if (!obj.isObject ())
    return;

  const auto& global = obj["g"];
  if (global.isArray ())
    SetSignerList (db, name, nullptr, global);

  const auto& apps = obj["a"];
  if (apps.isObject ())
    for (auto it = apps.begin (); it != apps.end (); ++it)
      {
        CHECK (it.key ().isString ());
        const std::string application = it.key ().asString ();

        if (!it->isArray ())
          {
            LOG (WARNING)
                << "Signer update for " << name
                << " and application " << application
                << " is not an array";
            continue;
          }

        SetSignerList (db, name, &application, *it);
      }
}

void
MoveProcessor::HandleAddressUpdate (const std::string& name,
                                    const Json::Value& obj)
{
  if (!obj.isObject ())
    return;

  auto* stmtDel = db.PrepareStatement (R"(
    DELETE FROM `addresses`
      WHERE `name` = ?1 AND `key` = ?2
  )");
  BindParameter (stmtDel, 1, name);

  auto* stmtIns = db.PrepareStatement (R"(
    INSERT OR REPLACE INTO `addresses`
      (`name`, `key`, `address`)
      VALUES (?1, ?2, ?3)
  )");
  BindParameter (stmtIns, 1, name);

  for (auto it = obj.begin (); it != obj.end (); ++it)
    {
      CHECK (it.key ().isString ());
      const auto key = it.key ().asString ();

      const auto val = *it;
      if (val.isNull ())
        {
          sqlite3_reset (stmtDel);
          BindParameter (stmtDel, 2, key);
          CHECK_EQ (sqlite3_step (stmtDel), SQLITE_DONE);
          VLOG (1)
              << "Deleted address association for " << name << " and " << key;
          continue;
        }
      if (val.isString ())
        {
          const auto addr = val.asString ();
          sqlite3_reset (stmtIns);
          BindParameter (stmtIns, 2, key);
          BindParameter (stmtIns, 3, addr);
          CHECK_EQ (sqlite3_step (stmtIns), SQLITE_DONE);
          VLOG (1)
              << "New address for " << name << " and " << key
              << ": " << addr;
          continue;
        }

      LOG (WARNING)
          << "Invalid address association for " << name << " and " << key
          << ": " << val;
    }
}

void
MoveProcessor::ProcessOne (const Json::Value& obj)
{
  CHECK (obj.isObject ());
  VLOG (1) << "Processing move:\n" << obj;

  const auto& nmVal = obj["name"];
  CHECK (nmVal.isString ());
  const std::string name = nmVal.asString ();

  CHECK (obj.isMember ("move"));
  const auto& mv = obj["move"];
  if (!mv.isObject ())
    {
      LOG (WARNING) << "Move by " << name << " is not an object:\n" << mv;
      return;
    }

  HandleSignerUpdate (name, mv["s"]);
  HandleAddressUpdate (name, mv["ca"]);
}

void
MoveProcessor::ProcessAll (const Json::Value& arr)
{
  LOG (INFO) << "Processing " << arr.size () << " moves";

  CHECK (arr.isArray ());
  for (const auto& entry : arr)
    ProcessOne (entry);
}

} // namespace xid
