// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "time.hpp"

namespace xid
{

/* The implementation here assumes that std::time_t *is* the Unix timestamp,
   which is true on most systems (including POSIX compliant ones).  The unit
   tests verify that this is really the case.  */

uint64_t
TimeToUnix (const std::time_t t)
{
  return t;
}

std::time_t
TimeFromUnix (const uint64_t u)
{
  return u;
}

} // namespace xid
