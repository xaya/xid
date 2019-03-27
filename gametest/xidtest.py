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
    the "data" field.
    """

    return self.getCustomState ("data", method, **kwargs)

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
