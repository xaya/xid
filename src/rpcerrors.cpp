// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcerrors.hpp"

#include <jsonrpccpp/common/exception.h>

namespace xid
{

void
ThrowJsonError (const ErrorCode code, const std::string& msg)
{
  throw jsonrpc::JsonRpcException (static_cast<int> (code), msg);
}

} // namespace xid
