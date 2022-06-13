#!/usr/bin/env python3
# coding=utf8

# Copyright (C) 2019-2022 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidauth import EjabberdXidAuth
from xidtest import XidTest

import codecs
import logging
import os
import os.path
import struct
import subprocess
import time

"""
Tests the xidauth script connected to a real xid and Xaya Core daemon.
"""


class XidAuthTest (XidTest):

  def createPassword (self, name):
    """
    Creates a Xid password for the given XMPP name, signed by the predefined
    address for this instance.
    """

    xayaName = self.decodeName (name)
    authMsg = self.rpc.game.getauthmessage (name=xayaName, application=self.app,
                                            data={})

    signed = self.rpc.xaya.signmessage (self.addr, authMsg["authmessage"])
    pwd = self.rpc.game.setauthsignature (password=authMsg["password"],
                                          signature=signed)

    return pwd

  def hexEncodeName (self, name):
    """
    Performs hex-encoding of the given Xaya name.  This always hex-encodes
    and thus does not work for simple names.
    """

    utf8Bytes = codecs.encode (name, "utf-8")
    hexStr = codecs.encode (utf8Bytes, "hex").decode ("ascii")

    return "x-" + hexStr

  def decodeName (self, name):
    """
    Decodes an XMPP name to the corresponding Xaya name.  This is just a simple
    wrapper around EjabberdXidAuth.decodeXmppName.
    """

    res = self.auth.decodeXmppName (name)
    assert res is not None

    return res

  def expectResult (self, parts, expectedOk):
    """
    Sends a command (consisting of a string array) to the xidauth process
    and verifies that the boolean result is as expected.
    """

    cmdStr = ":".join (parts)
    self.log.debug ("Sending command: %s" % cmdStr)

    length = struct.pack (">h", len (cmdStr))
    self.proc.stdin.write (length)
    self.proc.stdin.write (cmdStr.encode ("ascii"))
    self.proc.stdin.flush ()

    res = self.proc.stdout.read (4)
    (n, ok) = struct.unpack (">hh", res)
    self.assertEqual (n, 2)
    assert ok in [0, 1]
    self.log.debug ("Got response: %d" % ok)

    boolResult = (ok == 1)
    self.assertEqual (boolResult, expectedOk)

  def runTests (self):
    """
    Runs the main tests, after everything is set up and the xidauth
    process started already.
    """

    # Unsupported method.
    self.expectResult (["foo", "bar"], False)

    # Invalid encoded names.
    self.expectResult (["isuser", "invalid name", self.server], False)
    self.expectResult (["auth", "invalid name", self.server, "pwd"], False)

    # Wrong server given.
    pwd = self.createPassword ("domob")
    self.expectResult (["isuser", "domob", "other.server"], False)
    self.expectResult (["auth", "domob", "other.server", pwd], False)

    # Name that has signers but none valid for the server.
    pwd = self.createPassword ("nosigner")
    self.expectResult (["isuser", "nosigner", self.server], False)
    self.expectResult (["auth", "nosigner", self.server, pwd], False)

    # Check that all test names work fine.
    for n in self.testNames:
      self.log.info ("Testing name %s..." % self.decodeName (n))
      self.expectResult (["isuser", n, self.server], True)
      pwd = self.createPassword (n)
      self.expectResult (["auth", n, self.server, pwd], True)

  def run (self):
    self.generate (101)

    self.server = "server"
    self.app = "app"

    # We need an instance of EjabberdXidAuth for decoding names.  Create it
    # and then only use it for that purpose.  (This is not the instance
    # that will be used in the real test.)
    self.auth = EjabberdXidAuth ({}, None)
    self.auth.log = logging.getLogger ("xidauth")

    # Define some test names (of various types) and set up signer addresses
    # for them.  We use a single address for everyone for simplicity.
    self.mainLogger.info ("Setting up xid signers...")
    self.testNames = [
      "domob",
      self.hexEncodeName (""),
      self.hexEncodeName ("Foo Bar"),
      self.hexEncodeName (u"äöü"),
    ]
    self.addr = self.rpc.xaya.getnewaddress ("", "legacy")
    self.log.info ("Using signer address: %s" % self.addr)
    for n in self.testNames:
      xayaName = self.decodeName (n)
      self.sendMove (xayaName, {"s": {"g": [self.addr]}})

    # Also define a name with a signer that is not valid for the server.
    self.sendMove ("nosigner", {
      "s":
        {
          "a": {"other": [self.addr]},
        },
    })

    self.generate (1)

    # Verify that the signers are set correctly (just in case).
    for n in self.testNames:
      xayaName = self.decodeName (n)
      self.assertEqual (self.getRpc ("getnamestate", name=xayaName)["signers"],
        [{"addresses": [self.addr]}],
      )
    self.assertEqual (self.getRpc ("getnamestate", name="nosigner")["signers"],
      [
        {
          "application": "other",
          "addresses": [self.addr],
        },
      ]
    )

    # Now we can perform the main test with a running xidauth script.
    self.mainLogger.info ("Starting xidauth script...")
    srcdir = os.getenv ("srcdir")
    if srcdir is None:
      srcdir = "."
    binary = os.path.join (srcdir, "xidauth.py")
    cmd = [binary]
    rpcUrl = "http://localhost:%s" % self.gamenode.port
    cmd.extend (["--servers", "%s,%s,%s" % (self.server, self.app, rpcUrl)])
    cmd.append ("--logfile=%s" % os.path.join (self.basedir, "xidauth.log"))
    try:
      self.log.info ("Starting process: %s" % " ".join (cmd))
      self.proc = subprocess.Popen (cmd, stdin=subprocess.PIPE,
                                    stdout=subprocess.PIPE)
      time.sleep (1)
      rc = self.proc.poll ()
      if rc is not None:
        self.log.error ("xidauth process returned with code %d" % rc)
        raise AssertionError ("xidauth process died")
      self.log.info ("Started xidauth process successfully")
      self.runTests ()
    finally:
      self.log.info ("Cleaning up xidauth process...")
      if self.proc.poll () is None:
        self.proc.terminate ()
      self.proc.wait ()


if __name__ == "__main__":
  XidAuthTest ().main ()
