# Copyright (C) 2019-2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xayax.gametest import XayaXGameTest

import os
import os.path


class XidTest (XayaXGameTest):
  """
  Integration test for the Xid game-state daemon.
  """

  def __init__ (self):
    top_builddir = os.getenv ("top_builddir")
    if top_builddir is None:
      top_builddir = ".."
    binary = os.path.join (top_builddir, "src", "xid")
    super (XidTest, self).__init__ ("id", binary)

  def runBaseChainEnvironment (self):
    return self.runXayaXEthEnvironment ()

  def getRpc (self, method, **kwargs):
    """
    Calls the given "read-type" RPC method on the game daemon and returns
    the "data" field.
    """

    return self.getCustomState ("data", method, **kwargs)

  def createPassword (self, name, app, addr, expiry, extra):
    """
    Creates a Xid password for the given data, signed with the given
    Xaya address.
    """

    d = {
      "expiry": expiry,
      "extra": extra,
    }
    authMsg = self.rpc.game.getauthmessage (name=name, application=app, data=d)

    signed = self.env.signMessage (addr, authMsg["authmessage"])
    pwd = self.rpc.game.setauthsignature (password=authMsg["password"],
                                          signature=signed)

    return pwd
