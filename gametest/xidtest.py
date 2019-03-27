# Copyright (C) 2019 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xayagametest.testcase import XayaGameTest

import os
import os.path


class XidTest (XayaGameTest):
  """
  Integration test for the Xid game-state daemon.
  """

  def __init__ (self):
    top_builddir = os.getenv ("top_builddir")
    if top_builddir is None:
      top_builddir = ".."
    binary = os.path.join (top_builddir, "src", "xid")
    super (XidTest, self).__init__ ("id", binary)

  def getRpc (self, method, **kwargs):
    """
    Calls the given "read-type" RPC method on the game daemon and returns
    the "data" field.
    """

    return self.getCustomState ("data", method, **kwargs)
