#!/usr/bin/env python3
# coding=utf8

# Copyright (C) 2019-2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from ejabberd_xidauth import EjabberdXidAuth
from xidtest import XidTest

from xidauth.credentials import Credentials, Protocol
from xidauth.delegation import Encoder, Verifier

import codecs
import copy
import json
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

  def __init__ (self):
    super ().__init__ ()
    self.server = "server"
    self.app = "app"

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

  def getConfigJson (self):
    """
    Returns the ejabberd_xidauth config JSON to use for the test.
    """

    raise RuntimeError ("getConfigJson is not implemented")

  def runTests (self):
    """
    Runs the main tests, after everything is set up and the xidauth
    process started already.
    """

    raise RuntimeError ("runTests is not implemented")

  def run (self):
    # We need an instance of EjabberdXidAuth for decoding names.  Create it
    # and then only use it for that purpose.  (This is not the instance
    # that will be used in the real test.)
    self.auth = EjabberdXidAuth ({}, None)
    self.auth.log = logging.getLogger ("xidauth")

    # Now we can perform the main test with a running xidauth script.
    self.mainLogger.info ("Starting xidauth script...")
    srcdir = os.getenv ("srcdir")
    if srcdir is None:
      srcdir = "."
    binary = os.path.join (srcdir, "ejabberd_xidauth.py")
    cmd = [binary]
    cmd.append ("--logfile=%s" % os.path.join (self.basedir, "xidauth.log"))
    cmd.append ("--debug")
    config = self.getConfigJson ()
    # We want to preserve the environment (such as PYTHONPATH)
    # and only just set the xidauth config in it.
    env = copy.deepcopy (os.environ)
    env["EJABBERD_XIDAUTH_CONFIG"] = json.dumps (config)
    try:
      self.log.info ("Starting process: %s" % " ".join (cmd))
      self.proc = subprocess.Popen (cmd, env=env,
                                    stdin=subprocess.PIPE,
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


class XidGspAuthTest (XidAuthTest):

  def createPassword (self, name):
    """
    Creates a Xid password for the given XMPP name, signed by the predefined
    address for this instance.
    """

    xayaName = self.decodeName (name)
    return super ().createPassword (xayaName, self.app, self.addr)

  def getConfigJson (self):
    return {
      self.server: {
        "app": self.app,
        "xid-gsp": f"http://localhost:{self.gamenode.port}",
      },
    }

  def runTests (self):
    self.mainLogger.info ("Testing with XID GSP...")

    # Define some test names (of various types) and set up signer addresses
    # for them.  We use a single address for everyone for simplicity.
    self.mainLogger.info ("Setting up xid signers...")
    testNames = [
      "domob",
      self.hexEncodeName (""),
      self.hexEncodeName ("Foo Bar"),
      self.hexEncodeName (u"äöü"),
    ]
    self.addr = self.env.createSignerAddress ()
    self.log.info ("Using signer address: %s" % self.addr)
    for n in testNames:
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
    for n in testNames:
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
    for n in testNames:
      self.log.info ("Testing name %s..." % self.decodeName (n))
      self.expectResult (["isuser", n, self.server], True)
      pwd = self.createPassword (n)
      self.expectResult (["auth", n, self.server, pwd], True)


class DelegationContractAuthTest (XidAuthTest):

  def createPassword (self, name):
    """
    Creates a Xid password for the given XMPP name, signed by the predefined
    address for this instance.
    """

    xayaName = self.decodeName (name)
    cred = Credentials (xayaName, self.app)
    cred.protocol = Protocol.DELEGATION_CONTRACT

    msg = self.encoder.encodeCredentials (cred)
    acc = self.env.lookupSignerAccount (self.addr)
    signature = acc.sign_message (msg)
    cred.raw_signature = signature.signature

    return cred.password.decode ("ascii")

  def getConfigJson (self):
    return {
      self.server: {
        "app": self.app,
        "delegation-contract": {
          "evm-rpc": self.env.evm.rpcurl,
          "del-addr": self.contracts.delegation.address,
        },
      },
    }

  def setup (self):
    # In the general test setup, we deploy the delegation contract
    # to the test environment.  We need to do this here rather than
    # in runTests() so that we can provide it already with getConfigJson
    # to the authenticator in the ejabberd_xidauth script that is started.
    abi = Verifier.loadAbi ("XayaDelegation")
    self.contracts.delegation = self.env.evm.deployContract (
        self.env.contracts.account, abi,
        self.contracts.registry.address, "0x" + "00" * 20)

    self.encoder = Encoder (self.env.evm.w3.eth.chain_id,
                            self.contracts.delegation.address)

  def runTests (self):
    self.mainLogger.info ("Testing with delegation contract...")

    # Define test names and set up a signer address with permission for them.
    self.mainLogger.info ("Setting up test names and a signer address...")
    testNames = [
      "domob",
      self.hexEncodeName (""),
      self.hexEncodeName ("Foo Bar"),
      self.hexEncodeName (u"äöü"),
    ]
    noExpiry = 2**256 - 1
    self.addr = self.env.createSignerAddress ()
    self.log.info (f"Using signer address: {self.addr}")
    for n in testNames:
      xayaName = self.decodeName (n)
      self.env.register ("p", xayaName)
      self.generate (1)
      self.contracts.delegation.functions \
          .grant ("p", xayaName, [], self.addr, noExpiry, False) \
          .transact ({"from": self.contracts.account})
      self.generate (1)

    # Unsupported method.
    self.expectResult (["foo", "bar"], False)

    # Invalid encoded names.
    self.expectResult (["isuser", "invalid name", self.server], False)
    self.expectResult (["auth", "invalid name", self.server, "pwd"], False)

    # Wrong server given.
    pwd = self.createPassword ("domob")
    self.expectResult (["isuser", "domob", "other.server"], False)
    self.expectResult (["auth", "domob", "other.server", pwd], False)

    # Invalid format for password (not base64).
    self.expectResult (["auth", "domob", self.server, "not base64"], False)

    # Check that all test names work fine.
    for n in testNames:
      self.log.info ("Testing name %s..." % self.decodeName (n))
      self.expectResult (["isuser", n, self.server], True)
      pwd = self.createPassword (n)
      self.expectResult (["auth", n, self.server, pwd], True)

    # This name is not registered at first, but we register it.  Our signing
    # address has still no permission, though.
    self.expectResult (["isuser", "andy", self.server], False)
    self.env.register ("p", "andy")
    self.generate (1)
    self.expectResult (["isuser", "andy", self.server], True)
    pwd = self.createPassword ("andy")
    self.expectResult (["auth", "andy", self.server, pwd], False)


if __name__ == "__main__":
  XidGspAuthTest ().main ()
  DelegationContractAuthTest ().main ()
