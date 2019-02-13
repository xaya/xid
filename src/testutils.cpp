// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "testutils.hpp"

#include <glog/logging.h>

namespace xid
{

bool
JsonEquals (const Json::Value& actual, const std::string& expected)
{
  std::istringstream in(expected);
  Json::Value expectedJson;
  in >> expectedJson;

  if (actual == expectedJson)
    return true;

  LOG (ERROR)
      << "Actual JSON:\n" << actual
      << "\n\nis not equal to:\n" << expectedJson;
  return false;
}

} // namespace xid
