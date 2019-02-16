// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "credentials.hpp"

#include "base64.hpp"
#include "time.hpp"

#include <glog/logging.h>

#include <sstream>

namespace xid
{

bool
Credentials::FromPassword (const std::string& pwd)
{
  std::string decoded;
  if (!DecodeBase64 (pwd, decoded))
    return false;

  if (!data.ParseFromString (decoded))
    {
      LOG (ERROR) << "Failed to parse AuthData from decoded password";
      return false;
    }

  return true;
}

std::string
Credentials::ToPassword () const
{
  CHECK (ValidateFormat ());

  std::string rawData;
  CHECK (data.SerializeToString (&rawData));

  return EncodeBase64 (rawData);
}

namespace
{

bool
IsAlphaNumeric (const char c)
{
  if (c >= '0' && c <= '9')
    return true;

  if (c >= 'A' && c <= 'Z')
    return true;
  if (c >= 'a' && c <= 'z')
    return true;

  return false;
}

bool
IsAlphaNumericOrDot (const std::string& str)
{
  for (const auto c : str)
    if (!IsAlphaNumeric (c) && c != '.')
      return false;
  return true;
}

} // anonymous namespace

bool
Credentials::ValidateFormat () const
{
  for (const auto c : username)
    if (c == '\n')
      {
        LOG (ERROR) << "Invalid username (contains newline): " << username;
        return false;
      }

  for (const auto c : application)
    if (!IsAlphaNumeric (c) && c != '.' && c != '/')
      {
        LOG (ERROR) << "Invalid application name: " <<  application;
        return false;
      }

  for (const auto& entry : data.extra ())
    {
      if (!IsAlphaNumericOrDot (entry.first))
        {
          LOG (ERROR) << "Invalid extra key: " << entry.first;
          return false;
        }
      if (!IsAlphaNumericOrDot (entry.second))
        {
          LOG (ERROR) << "Invalid extra value: " << entry.second;
          return false;
        }
    }

  return true;
}

std::string
Credentials::GetAuthMessage () const
{
  CHECK (ValidateFormat ());

  std::ostringstream out;

  out << "Xid login\n";
  out << username << "\n";
  out << "at: " << application << "\n";

  out << "expires: ";
  if (HasExpiry ())
    out << GetExpiry () << "\n";
  else
    out << "never\n";
  out << "extra:\n";

  const auto extra = GetExtra ();
  for (const auto& entry : extra)
    out << entry.first << "=" << entry.second << "\n";

  return out.str ();
}

bool
Credentials::IsExpired (const std::time_t at) const
{
  if (!HasExpiry ())
    return false;
  return at > GetExpiry ();
}

bool
Credentials::IsExpired () const
{
  return IsExpired (std::time (nullptr));
}

std::string
Credentials::GetSignature () const
{
  return EncodeBase64 (data.signature_bytes ());
}

void
Credentials::SetSignature (const std::string& sgn)
{
  CHECK (DecodeBase64 (sgn, *data.mutable_signature_bytes ()))
      << "The signature is not valid Base64: " << sgn;
}

std::time_t
Credentials::GetExpiry () const
{
  return TimeFromUnix (data.expiry ());
}

void
Credentials::SetExpiry (const std::time_t t)
{
  data.set_expiry (TimeToUnix (t));
}

void
Credentials::AddExtra (const std::string& key, const std::string& value)
{
  CHECK_EQ (data.extra ().count (key), 0);
  (*data.mutable_extra ())[key] = value;
}

Credentials::ExtraMap
Credentials::GetExtra () const
{
  return ExtraMap (data.extra ().begin (), data.extra ().end ());
}

} // namespace xid
