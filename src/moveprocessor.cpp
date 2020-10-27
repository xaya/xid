// Copyright (C) 2019-2020 The Xaya developers
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
SetSignerList (xaya::SQLiteDatabase& db, const std::string& name,
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

  xaya::SQLiteDatabase::Statement stmt;
  if (application == nullptr)
    stmt = db.Prepare (R"(
      DELETE FROM `signers`
        WHERE `name` = ?1 AND `application` IS NULL
    )");
  else
    {
      stmt = db.Prepare (R"(
        DELETE FROM `signers`
          WHERE `name` = ?1 AND `application` = ?2
      )");
      stmt.Bind (2, *application);
    }
  stmt.Bind (1, name);
  stmt.Execute ();

  stmt = db.Prepare (R"(
    INSERT INTO `signers`
      (`name`, `application`, `address`)
      VALUES (?1, ?2, ?3)
  )");
  stmt.Bind (1, name);
  if (application == nullptr)
    stmt.BindNull (2);
  else
    stmt.Bind (2, *application);

  for (const auto& addr : signerArr)
    {
      if (!addr.isString ())
        {
          LOG (WARNING)
              << "Signer value in update for " << name
              << " is not a string: " << addr;
          continue;
        }

      stmt.Bind (3, addr.asString ());
      stmt.Execute ();

      /* We reuse the prepared statement for all signer inserts, since they
         are just the same operation repeated.  We can even keep the bindings
         for name and application, and just override address in the next
         iteration of the loop.  */
      stmt.Reset ();
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

  auto stmtDel = db.Prepare (R"(
    DELETE FROM `addresses`
      WHERE `name` = ?1 AND `key` = ?2
  )");
  stmtDel.Bind (1, name);

  auto stmtIns = db.Prepare (R"(
    INSERT OR REPLACE INTO `addresses`
      (`name`, `key`, `address`)
      VALUES (?1, ?2, ?3)
  )");
  stmtIns.Bind (1, name);

  for (auto it = obj.begin (); it != obj.end (); ++it)
    {
      CHECK (it.key ().isString ());
      const auto key = it.key ().asString ();

      const auto val = *it;
      if (val.isNull ())
        {
          stmtDel.Reset ();
          stmtDel.Bind (2, key);
          stmtDel.Execute ();
          VLOG (1)
              << "Deleted address association for " << name << " and " << key;
          continue;
        }
      if (val.isString ())
        {
          const auto addr = val.asString ();
          stmtIns.Reset ();
          stmtIns.Bind (2, key);
          stmtIns.Bind (3, addr);
          stmtIns.Execute ();
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
