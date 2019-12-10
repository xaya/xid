// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gamestatejson.hpp"

#include <glog/logging.h>

#include <map>
#include <set>

namespace xid
{

namespace
{

/** Array of signer addresses.  */
using SignerArray = std::set<std::string>;

/**
 * Converts a SignerArray to a JSON array.
 */
Json::Value
SignerArrayToJson (const SignerArray& arr)
{
  Json::Value res(Json::arrayValue);
  for (const auto& s : arr)
    res.append (s);
  return res;
}

/**
 * Retrieves the signers value of a name as JSON.
 */
Json::Value
GetNameSigners (Database& db, const std::string& name)
{
  auto* stmt = db.PrepareStatement (R"(
    SELECT `application`, `address`
      FROM `signers`
      WHERE `name` = ?1
  )");
  BindParameter (stmt, 1, name);

  SignerArray globalSigners;
  std::map<std::string, SignerArray> signers;
  while (true)
    {
      const int rc = sqlite3_step (stmt);
      if (rc == SQLITE_DONE)
        break;
      CHECK_EQ (rc, SQLITE_ROW);

      SignerArray* arrayRef = nullptr;
      if (IsColumnNull (stmt, 0))
        arrayRef = &globalSigners;
      else
        arrayRef = &signers[GetColumnValue<std::string> (stmt, 0)];
      CHECK (arrayRef != nullptr);

      arrayRef->emplace (GetColumnValue<std::string> (stmt, 1));
    }

  Json::Value res(Json::arrayValue);
  if (!globalSigners.empty ())
    {
      Json::Value globalSignersJson(Json::objectValue);
      globalSignersJson["addresses"] = SignerArrayToJson (globalSigners);
      res.append (globalSignersJson);
    }
  for (const auto& entry : signers)
    {
      Json::Value curJson(Json::objectValue);
      curJson["application"] = entry.first;
      curJson["addresses"] = SignerArrayToJson (entry.second);
      res.append (curJson);
    }

  return res;
}

/**
 * Retrieves all address associations for a name as JSON.
 */
Json::Value
GetNameAddresses (Database& db, const std::string& name)
{
  auto* stmt = db.PrepareStatement (R"(
    SELECT `key`, `address`
      FROM `addresses`
      WHERE `name` = ?1
  )");
  BindParameter (stmt, 1, name);

  Json::Value res(Json::objectValue);
  while (true)
    {
      const int rc = sqlite3_step (stmt);
      if (rc == SQLITE_DONE)
        break;
      CHECK_EQ (rc, SQLITE_ROW);

      const auto key = GetColumnValue<std::string> (stmt, 0);
      const auto addr = GetColumnValue<std::string> (stmt, 1);

      CHECK (!res.isMember (key));
      res[key] = addr;
    }

  return res;
}

} // anonymous namespace

Json::Value
GetNameState (Database& db, const std::string& name)
{
  Json::Value res(Json::objectValue);
  res["name"] = name;
  res["signers"] = GetNameSigners (db, name);
  res["addresses"] = GetNameAddresses (db, name);

  return res;
}

Json::Value
GetFullState (Database& db)
{
  auto* stmt = db.PrepareStatement (R"(
    SELECT DISTINCT `name` FROM `signers`
    UNION SELECT DISTINCT `name` FROM `addresses`
  )");

  Json::Value names(Json::objectValue);
  while (true)
    {
      const int rc = sqlite3_step (stmt);
      if (rc == SQLITE_DONE)
        break;
      CHECK_EQ (rc, SQLITE_ROW);

      const auto name = GetColumnValue<std::string> (stmt, 0);
      CHECK (!names.isMember (name));
      names[name] = GetNameState (db, name);
    }

  Json::Value res(Json::objectValue);
  res["names"] = names;

  return res;
}

} // namespace xid
