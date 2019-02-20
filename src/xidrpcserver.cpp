// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xidrpcserver.hpp"

#include "gamestatejson.hpp"

#include "auth/credentials.hpp"
#include "auth/time.hpp"

#include <xayagame/uint256.hpp>

#include <glog/logging.h>

namespace xid
{

namespace
{

/**
 * Error codes returned from the RPC server.  All values should have an
 * explicit integer number, because this also defines the RPC protocol
 * itself for clients that do not have access to the ErrorCode enum
 * directly and only read the integer values.
 */
enum class ErrorCode
{

  /* Invalid values for arguments (e.g. passing a malformed JSON value for
     an object parameter or an out-of-range integer).  */
  INVALID_ARGUMENT = -1,

  /* The provided data (name, applcation, extra) is invalid while constructing
     an auth message (not validating a password).  */
  AUTH_INVALID_DATA = 1,
  /* An invalid password string was provided, which could not be decoded to
     a valid protocol buffer.  This is not thrown when validating a password,
     just when modifying it.  */
  AUTH_INVALID_PASSWORD = 2,

};

/**
 * Throws a JSON-RPC error from the current method.  This throws an exception,
 * so does not return to the caller in a normal way.
 */
void
ThrowJsonError (const ErrorCode code, const std::string& msg)
{
  throw jsonrpc::JsonRpcException (static_cast<int> (code), msg);
}

} // anonymous namespace

void
XidRpcServer::stop ()
{
  LOG (INFO) << "RPC method called: stop";
  game.RequestStop ();
}

Json::Value
XidRpcServer::getcurrentstate ()
{
  LOG (INFO) << "RPC method called: getcurrentstate";
  return game.GetCurrentJsonState ();
}

Json::Value
XidRpcServer::waitforchange ()
{
  LOG (INFO) << "RPC method called: waitforchange";

  xaya::uint256 block;
  game.WaitForChange (&block);

  /* If there is no best block so far, return JSON null.  */
  if (block.IsNull ())
    return Json::Value ();

  /* Otherwise, return the block hash.  */
  return block.ToHex ();
}

Json::Value
XidRpcServer::getnamestate (const std::string& name)
{
  LOG (INFO) << "RPC method called: getnamestate " << name;
  return logic.GetCustomStateData (game,
    [&name] (Database& db)
      {
        return GetNameState (db, name);
      });
}

namespace
{

/**
 * Parses the data JSON argument for auth RPCs and sets the corresponding
 * data in the Credentials instance.  Throws a JSON-RPC error if the data
 * object is invalid.
 */
void
ApplyAuthDataJson (const Json::Value& data, Credentials& cred)
{
  CHECK (data.isObject ());

  const auto& expiryVal = data["expiry"];
  if (!expiryVal.isNull ())
    {
      if (!expiryVal.isInt ())
        ThrowJsonError (ErrorCode::INVALID_ARGUMENT,
                        "expiry must be an integer");
      cred.SetExpiry (TimeFromUnix (expiryVal.asInt ()));
    }

  const auto& extraVal = data["extra"];
  if (!extraVal.isNull ())
    {
      if (!extraVal.isObject ())
        ThrowJsonError (ErrorCode::INVALID_ARGUMENT, "extra must be an object");
      for (auto i = extraVal.begin (); i != extraVal.end (); ++i)
        {
          CHECK (i.key ().isString ());
          if (!i->isString ())
            ThrowJsonError (ErrorCode::INVALID_ARGUMENT,
                            "extra value must be a string");
          cred.AddExtra (i.key ().asString (), i->asString ());
        }
    }
}

} // anonymous namespace

Json::Value
XidRpcServer::getauthmessage (const std::string& application,
                              const Json::Value& data,
                              const std::string& name)
{
  LOG (INFO)
      << "RPC method called: getauthmessage\n"
      << "  name: " << name << "\n"
      << "  application: " << application << "\n"
      << "  data: " << data;

  Credentials cred(name, application);
  ApplyAuthDataJson (data, cred);

  if (!cred.ValidateFormat ())
    ThrowJsonError (ErrorCode::AUTH_INVALID_DATA,
                    "the authentication data is invalid");

  Json::Value res(Json::objectValue);
  res["authmessage"] = cred.GetAuthMessage ();
  res["password"] = cred.ToPassword ();

  return res;
}

std::string
XidRpcServer::setauthsignature (const std::string& password,
                                const std::string& signature)
{
  LOG (INFO)
      << "RPC method called: setauthsignature\n"
      << "  password: " << password << "\n"
      << "  signature: " << signature;

  /* The name and application are not relevant for this, as they are not
     part of the password string in any way.  Thus we can just set dummy
     values for them.  */
  Credentials cred("dummy", "dummy");

  if (!cred.FromPassword (password))
    ThrowJsonError (ErrorCode::AUTH_INVALID_PASSWORD,
                    "failed to parse the password string");
  if (!cred.ValidateFormat ())
    ThrowJsonError (ErrorCode::AUTH_INVALID_DATA,
                    "the authentication data is invalid");

  cred.SetSignature (signature);

  return cred.ToPassword ();
}

namespace
{

/**
 * Returns true if the given address is authorised to sign for the given name
 * and application.
 */
bool
IsValidSigner (Database& db, const std::string& addr,
               const std::string& name, const std::string& app)
{
  auto* stmt = db.PrepareStatement (R"(
    SELECT `application`
      FROM `signers`
      WHERE `name` = ?1 AND `address` = ?2
  )");
  BindParameter (stmt, 1, name);
  BindParameter (stmt, 2, addr);

  while (true)
    {
      const int rc = sqlite3_step (stmt);
      if (rc == SQLITE_DONE)
        return false;
      CHECK_EQ (rc, SQLITE_ROW);

      /* If the application is NULL, we've found a global signer.  */
      if (IsColumnNull (stmt, 0))
        return true;

      /* Otherwise, check if it matches the requested application.  */
      if (GetColumnValue<std::string> (stmt, 0) == app)
        return true;
    }
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
    [this, &name, &application, &password] (Database& db)
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
        const auto verifyResult = xayaRo.verifymessage ("", authMsg,
                                                        cred.GetSignature ());
        CHECK (verifyResult.isObject ());
        if (!verifyResult["valid"].asBool ()
              || !IsValidSigner (db, verifyResult["address"].asString (),
                                 name, application))
          {
            VLOG (1) << "Failed signature verification: " << verifyResult;
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
