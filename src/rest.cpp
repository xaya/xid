// Copyright (C) 2019-2022 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rest.hpp"

#include "gamestatejson.hpp"

#include <microhttpd.h>

namespace xid
{

RestApi::SuccessResult
RestApi::Process (const std::string& url)
{
  SuccessResult res;
  if (HandleState (url, game, res))
    return res;
  if (HandleHealthz (url, game, res))
    return res;

  std::string remainder;
  if (MatchEndpoint (url, "/name/", remainder))
    {
      const Json::Value res = logic.GetCustomStateData (game,
        [&remainder] (const xaya::SQLiteDatabase& db)
          {
            return GetNameState (db, remainder);
          });
      return SuccessResult (res);
    }

  throw HttpError (MHD_HTTP_NOT_FOUND, "invalid API endpoint");
}

} // namespace xid
