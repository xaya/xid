// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XIDAUTH_BASE64_HPP
#define XIDAUTH_BASE64_HPP

#include <string>

namespace xid
{

/**
 * Encodes the given string (potentially with binary data) into Xid's
 * base64 format (as used for passwords).
 */
std::string EncodeBase64 (const std::string& data);

/**
 * Decodes the given string from Xid's base64 format to a string of binary data.
 * Returns false if the decoding failed because of invalid data.
 */
bool DecodeBase64 (const std::string& encoded, std::string& data);

} // namespace xid

#endif // XIDAUTH_BASE64_HPP
