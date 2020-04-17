// Copyright (C) 2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_NONSTATERPC_HPP
#define XID_NONSTATERPC_HPP

#include "auth/credentials.hpp"

#include <json/json.h>

#include <string>

namespace xid
{

/**
 * Implementation of the RPC methods that do not require a game state,
 * and are thus shared between the full GSP and the light-mode binary.
 */
class NonStateRpc
{

public:

  explicit NonStateRpc () = default;

  NonStateRpc (const NonStateRpc&) = delete;
  void operator= (const NonStateRpc&) = delete;

  /**
   * Parses the data JSON argument for auth RPCs and sets the corresponding
   * data in the Credentials instance.  Throws a JSON-RPC error if the data
   * object is invalid.
   */
  static void ApplyAuthDataJson (const Json::Value& data, Credentials& cred);

  Json::Value getauthmessage (const std::string& application,
                              const Json::Value& data,
                              const std::string& name) const;

  std::string setauthsignature (const std::string& password,
                                const std::string& signature) const;

};

} // namespace xid

#endif // XID_NONSTATERPC_HPP
