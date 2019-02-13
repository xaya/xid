// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_TESTUTILS_HPP
#define XID_TESTUTILS_HPP

#include <json/json.h>

#include <string>

namespace xid
{

/**
 * Compares the given JSON to the expected value, which is given as string
 * and parsed into JSON.
 */
bool JsonEquals (const Json::Value& actual, const std::string& expected);

} // namespace xid

#endif // XID_TESTUTILS_HPP
