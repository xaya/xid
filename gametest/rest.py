#!/usr/bin/env python3
# coding=utf8

# Copyright (C) 2019-2020 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

"""
Tests the REST API for xid.
"""

from xidtest import XidTest

import codecs
import json
import urllib.error
import urllib.parse
import urllib.request


class GetNameStateTest (XidTest):

  def expectError (self, code, path, **kwargs):
    """
    Sends a request using urlopen to the given path (and with optional
    extra arguments) and expects a given HTTP error code.
    """

    try:
      url = "http://localhost:%d%s" % (self.restPort, path)
      urllib.request.urlopen (url, **kwargs)
      raise AssertionError ("expected HTTP error")
    except urllib.error.HTTPError as exc:
      self.assertEqual (exc.code, code)

  def run (self):
    self.generate (101)
    addr = self.rpc.xaya.getnewaddress ("", "legacy")
    self.sendMove ("domob", {"s": {"g": [addr]}})
    self.generate (1)

    self.mainLogger.info ("Enabling the REST interface...")
    self.stopGameDaemon ()
    self.restPort = self.basePort + 10
    self.log.info ("Using port %d for the REST API" % self.restPort)
    self.startGameDaemon (extraArgs=["--rest_port=%d" % self.restPort])

    self.mainLogger.info ("Testing error cases...")
    self.expectError (405, "/state", data=b"POST data")
    self.expectError (404, "")
    self.expectError (404, "/")
    self.expectError (404, "/foo")
    self.expectError (404, "/state/")

    self.mainLogger.info ("Testing name retrieval...")
    for name in ["domob", "foo/bar", "", "abc def", u"kräfti"]:
      encoded = urllib.parse.quote (codecs.encode (name, "utf-8"))
      url = "http://localhost:%d/name/%s" % (self.restPort, encoded)
      resp = urllib.request.urlopen (url)
      self.assertEqual (resp.getcode (), 200)
      res = json.loads (resp.read ())
      self.assertEqual (res["data"]["name"], name)
      self.assertEqual (res, self.rpc.game.getnamestate (name=name))


if __name__ == "__main__":
  GetNameStateTest ().main ()
