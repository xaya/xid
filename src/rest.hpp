// Copyright (C) 2019-2020 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_REST_HPP
#define XID_REST_HPP

#include "logic.hpp"

#include <xayagame/game.hpp>
#include <xayagame/rest.hpp>

namespace xid
{

/**
 * HTTP server providing a REST API for reading xid data.
 */
class RestApi : public xaya::RestApi
{

private:

  /** The underlying Game instance that manages everything.  */
  xaya::Game& game;

  /** The game logic implementation.  */
  XidGame& logic;

protected:

  SuccessResult Process (const std::string& url) override;

public:

  explicit RestApi (xaya::Game& g, XidGame& l, const int p)
    : xaya::RestApi(p), game(g), logic(l)
  {}

};

} // namespace xid

#endif // XID_REST_HPP
