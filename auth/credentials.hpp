// Copyright (C) 2019-2022 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XIDAUTH_CREDENTIALS_HPP
#define XIDAUTH_CREDENTIALS_HPP

#include "auth.pb.h"

#include <ctime>
#include <map>
#include <string>

namespace xid
{

/**
 * A set of Xid authentication credentials.  This is the main interface
 * to libxidauth.  Such credentials can be constructed directly (e.g. when
 * an application is building them up for constructing a password out
 * of them), or they can be created by parsing an existing password and
 * then validating the data.
 */
class Credentials
{

private:

  /** The username for which the credentials are.  */
  const std::string username;
  /** The application name for which the credentials are.  */
  const std::string application;

  /** The other authentication data in the password protocol buffer.  */
  AuthData data;

public:

  explicit Credentials (const std::string& n, const std::string& a)
    : username(n), application(a)
  {}

  Credentials () = delete;
  Credentials (const Credentials&) = delete;
  void operator= (const Credentials&) = delete;

  /**
   * Parses a given password string to fill in the auth data.  Returns false
   * if parsing fails.  The data itself is not validated, so even if this
   * method returns true, it may be invalid (e.g. invalid extra key/value
   * strings).
   */
  bool FromPassword (const std::string& pwd);

  /**
   * Converts the current authentication data in this instance to a password
   * string and returns it.
   */
  std::string ToPassword () const;

  /**
   * Validates the authentication data.  Returns true if username, application
   * and authentication data all follow the expected format for all strings.
   * This does not verify the expiration time, nor does it verify that the
   * signature (if present) is correct.
   */
  bool ValidateFormat () const;

  /**
   * Returns the authentication message, which is the string that has to be
   * signed with the Xaya address.  This function must not be called if the
   * data is invalid (as returned by ValidateFormat()).
   */
  std::string GetAuthMessage () const;

  /**
   * Returns true if the credentials are expired at the given time.
   */
  bool IsExpired (std::time_t at) const;

  /**
   * Returns true if the credentials are expired now.
   */
  bool IsExpired () const;

  /**
   * Returns the signature contained in the protocol buffer, encoded
   * as base64 (per Xaya Core for verifymessage).
   */
  std::string GetSignature () const;

  /**
   * Sets the signature field in the protocol buffer.  The sgn argument
   * must be base64 encoded, and will be set as raw bytes inside
   * the proto.
   */
  void SetSignature (const std::string& sgn);

  /* Accessor functions for the data in the protocol buffer.  */

  bool
  HasExpiry () const
  {
    return data.has_expiry ();
  }

  std::time_t GetExpiry () const;
  void SetExpiry (std::time_t);

  using ExtraMap = std::map<std::string, std::string>;

  void AddExtra (const std::string& key, const std::string& value);
  ExtraMap GetExtra () const;

};

} // namespace xid

#endif // XIDAUTH_CREDENTIALS_HPP
