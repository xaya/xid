// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XIDAUTH_TIME_HPP
#define XIDAUTH_TIME_HPP

#include <cstdint>
#include <ctime>

namespace xid
{

/**
 * Converts a std::time_t instance to a Unix timestamp.
 */
uint64_t TimeToUnix (std::time_t t);

/**
 * Converts a Unix timestamp to a std::time_t instance.
 */
std::time_t TimeFromUnix (uint64_t u);

} // namespace xid

#endif // XIDAUTH_TIME_HPP
