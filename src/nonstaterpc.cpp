// Copyright (C) 2019-2025 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "nonstaterpc.hpp"

#include "rpcerrors.hpp"

#include "auth/time.hpp"

#include <xayautil/base64.hpp>

#include <glog/logging.h>

namespace xid
{

void
NonStateRpc::ApplyAuthDataJson (const Json::Value& data, Credentials& cred)
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

  const auto& protocolVal = data["protocol"];
  if (!protocolVal.isNull ())
    {
      if (!protocolVal.isString ())
        ThrowJsonError (ErrorCode::INVALID_ARGUMENT,
                        "protocol must be a string");

      const auto proto = protocolVal.asString ();
      if (proto == "xid-gsp")
        cred.SetProtocol (Protocol::XID_GSP);
      else if (proto == "delegation-contract")
        cred.SetProtocol (Protocol::DELEGATION_CONTRACT);
      else
        ThrowJsonError (ErrorCode::INVALID_ARGUMENT, "invalid protocol value");
    }
}

Json::Value
NonStateRpc::getauthmessage (const std::string& application,
                             const Json::Value& data,
                             const std::string& name) const
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
NonStateRpc::setauthsignature (const std::string& password,
                               const std::string& signature) const
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

  std::string rawSignature;
  if (!xaya::DecodeBase64 (signature, rawSignature))
    ThrowJsonError (ErrorCode::AUTH_INVALID_SIGNATURE,
                    "the signature is not base64");

  cred.SetSignature (signature);

  return cred.ToPassword ();
}

} // namespace xid
