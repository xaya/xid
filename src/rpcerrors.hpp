// Copyright (C) 2019-2022 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_RPCERRORS_HPP
#define XID_RPCERRORS_HPP

#include <string>

namespace xid
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
  /* The Xaya wallet would be needed but is not enabled.  */
  WALLET_NOT_ENABLED = -2,
  /* The Xaya wallet is locked.  */
  WALLET_LOCKED = -3,
  /* This method is considered unsafe and not enabled in the server.  */
  UNSAFE_METHOD = -4,

  /* The provided data (name, applcation, extra) is invalid while constructing
     an auth message (not validating a password).  */
  AUTH_INVALID_DATA = 1,
  /* An invalid password string was provided, which could not be decoded to
     a valid protocol buffer.  This is not thrown when validating a password,
     just when modifying it.  */
  AUTH_INVALID_PASSWORD = 2,
  /* The Xaya wallet does not hold any key allowed to sign the credentials.  */
  AUTH_NO_KEY = 3,

};

/**
 * Throws a JSON-RPC error from the current method.  This throws an exception,
 * so does not return to the caller in a normal way.
 */
void ThrowJsonError (const ErrorCode code, const std::string& msg);

} // namespace xid

#endif // XID_RPCERRORS_HPP
