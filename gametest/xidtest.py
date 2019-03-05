# Copyright (C) 2019 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xayagametest.testcase import XayaGameTest

import os
import os.path
import re

from jsonrpclib import ProtocolError


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
    the "data" field.  This also makes sure to wait until the game is
    synced to the current best block, just like getGameState() does.
    """

    # Ensure we are synced.
    self.getGameState ()

    # Actually call the method.
    fcn = getattr (self.rpc.game, method)
    res = fcn (**kwargs)

    # Verify that we are really synced.
    self.assertEqual (res["state"], "up-to-date")
    self.assertEqual (res["blockhash"], self.rpc.xaya.getbestblockhash ())

    # Extract data member.
    return res["data"]

  def assertEqual (self, a, b):
    """
    Asserts that two values are equal, logging them if not.
    """

    if a == b:
      return

    self.log.error ("The value of:\n%s\n\nis not equal to:\n%s" % (a, b))
    raise AssertionError ("%s != %s" % (a, b))

  def expectError (self, code, msgRegExp, method, *args, **kwargs):
    """
    Calls the method object with the given arguments, and expects that
    an RPC error is raised matching the code and message.
    """

    try:
      method (*args, **kwargs)
      self.log.error ("Expected RPC error with code=%d and message %s"
                        % (code, msgRegExp))
      raise AssertionError ("expected RPC error was not raised")
    except ProtocolError as exc:
      self.log.info ("Caught expected RPC error: %s" % exc)
      (c, m) = exc.args[0]
      self.assertEqual (c, code)
      msgPattern = re.compile (msgRegExp)
      assert msgPattern.match (m)
