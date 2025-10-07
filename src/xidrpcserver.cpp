// Copyright (C) 2019-2025 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xidrpcserver.hpp"

#include "gamestatejson.hpp"
#include "rpcerrors.hpp"

#include "auth/credentials.hpp"
#include "auth/time.hpp"

#include <xayagame/gamerpcserver.hpp>

#include <glog/logging.h>

namespace xid
{

void
XidRpcServer::EnsureUnsafeAllowed (const std::string& method) const
{
  if (!unsafeMethods)
    {
      LOG (WARNING) << "Blocked unsafe '" << method << "' call";
      ThrowJsonError (ErrorCode::UNSAFE_METHOD,
                      "unsafe RPC methods are disabled in the server");
    }
}

void
XidRpcServer::EnableUnsafeMethods ()
{
  LOG (WARNING) << "Enabling unsafe RPC methods";
  unsafeMethods = true;
}

void
XidRpcServer::stop ()
{
  LOG (INFO) << "RPC method called: stop";
  EnsureUnsafeAllowed ("stop");
  game.RequestStop ();
}

Json::Value
XidRpcServer::getcurrentstate ()
{
  LOG (INFO) << "RPC method called: getcurrentstate";
  EnsureUnsafeAllowed ("getcurrentstate");
  return game.GetCurrentJsonState ();
}

Json::Value
XidRpcServer::getnullstate ()
{
  LOG (INFO) << "RPC method called: getnullstate";
  return game.GetNullJsonState ();
}

std::string
XidRpcServer::waitforchange (const std::string& knownBlock)
{
  LOG (INFO) << "RPC method called: waitforchange " << knownBlock;
  return xaya::GameRpcServer::DefaultWaitForChange (game, knownBlock);
}

Json::Value
XidRpcServer::getnamestate (const std::string& name)
{
  LOG (INFO) << "RPC method called: getnamestate " << name;
  return logic.GetCustomStateData (game,
    [&name] (const xaya::SQLiteDatabase& db)
      {
        return GetNameState (db, name);
      });
}

Json::Value
XidRpcServer::getauthmessage (const std::string& application,
                              const Json::Value& data,
                              const std::string& name)
{
  return nonState.getauthmessage (application, data, name);
}

std::string
XidRpcServer::setauthsignature (const std::string& password,
                                const std::string& signature)
{
  return nonState.setauthsignature (password, signature);
}

namespace
{

/**
 * Returns true if the given address is authorised to sign for the given name
 * and application.
 */
bool
IsValidSigner (const xaya::SQLiteDatabase& db, const std::string& addr,
               const std::string& name, const std::string& app)
{
  auto stmt = db.PrepareRo (R"(
    SELECT `application`
      FROM `signers`
      WHERE `name` = ?1 AND `address` = ?2
  )");
  stmt.Bind (1, name);
  stmt.Bind (2, addr);

  while (stmt.Step ())
    {
      /* If the application is NULL, we've found a global signer.  */
      if (stmt.IsNull (0))
        return true;

      /* Otherwise, check if it matches the requested application.  */
      if (stmt.Get<std::string> (0) == app)
        return true;
    }

  return false;
}

} // anonymous namespace

Json::Value
XidRpcServer::verifyauth (const std::string& application,
                          const std::string& name,
                          const std::string& password)
{
  LOG (INFO)
      << "RPC method called: verifyauth\n"
      << "  name: " << name << "\n"
      << "  application: " << application << "\n"
      << "  password: " << password;
  return logic.GetCustomStateData (game,
    [this, &name, &application, &password] (const xaya::SQLiteDatabase& db)
      {
        Json::Value res(Json::objectValue);
        res["valid"] = false;

        Credentials cred(name, application);
        if (!cred.FromPassword (password))
          {
            res["state"] = "malformed";
            return res;
          }

        if (!cred.ValidateFormat ())
          {
            res["state"] = "invalid-data";
            return res;
          }

        res["expiry"]
            = cred.HasExpiry ()
                ? static_cast<Json::Int64> (TimeToUnix (cred.GetExpiry ()))
                : Json::Value ();

        const auto& extraMap = cred.GetExtra ();
        Json::Value extra(Json::objectValue);
        for (const auto& entry : extraMap)
          extra[entry.first] = entry.second;
        CHECK_EQ (extraMap.size (), extra.size ());
        res["extra"] = extra;

        const std::string authMsg = cred.GetAuthMessage ();
        const std::string sgnAddr = logic.VerifyMessage (authMsg,
                                                         cred.GetSignature ());
        if (!IsValidSigner (db, sgnAddr, name, application))
          {
            VLOG (1) << "Not a valid signer address: " << sgnAddr;
            res["state"] = "invalid-signature";
            return res;
          }

        /* The check for being expired is the last thing done.  This ensures
           that an "expired" state means that all else is good, and that the
           credentials are really "ok except for expiry".  Together with the
           returned "expiry" field, this allows client applications to
           re-evaluate expiry if they want (e.g. if the current system time
           may not be correct or applicable).  */
        if (cred.IsExpired ())
          {
            res["state"] = "expired";
            return res;
          }

        res["state"] = "valid";
        res["valid"] = true;
        return res;
      });
}

} // namespace xid
