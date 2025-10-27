#!/usr/bin/env python3

# Copyright (C) 2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidauth.credentials import Credentials, InvalidCredentialsError, Protocol
from xidauth.delegation import Encoder, Verifier

from xidtest import XidTest

import binascii
import sys
import time
import unittest

################################################################################

class EncoderTest (unittest.TestCase):

  ADDR1 = "0xEB4c2EF7874628B646B8A59e4A309B94e14C2a6B"
  ADDR2 = "0xD5fd31CfD529498B1668fE9dFa336c475AC57C76"

  def testGoldenData (self):
    c = Credentials ("name", "app")
    c.expiry = 123
    c.protocol = Protocol.DELEGATION_CONTRACT
    c.extra["foo"] = "bar"
    c.extra["abc"] = "def"

    enc = Encoder (137, self.ADDR1)
    msg = enc.encodeCredentials (c)

    self.assertEqual (msg.version, b"\x01")
    self.assertEqual (
        msg.header,
        binascii.unhexlify (
            "7b1448ef6131957018ab89a7dceebb1560c5b38e2734ba37f18a82d00415a624"))
    self.assertEqual (
        msg.body,
        binascii.unhexlify (
            "f5ceadeccbac2325db615f77a2a73280ff0b1c5c0e49587d578d9ba0eeecd01b"))

  def testHeaderCommitment (self):
    c = Credentials ("name", "app")
    c.expiry = 123

    msg1 = Encoder (137, self.ADDR1).encodeCredentials (c)
    msg2 = Encoder (137, self.ADDR2).encodeCredentials (c)
    msg3 = Encoder (100, self.ADDR1).encodeCredentials (c)

    self.assertNotEqual (msg1.header, msg2.header)
    self.assertNotEqual (msg1.header, msg3.header)
    self.assertNotEqual (msg2.header, msg3.header)
    self.assertEqual (msg1.body, msg2.body)
    self.assertEqual (msg1.body, msg3.body)

  def testBodyCommitment (self):
    enc = Encoder (137, self.ADDR1)

    c = Credentials ("name", "app")
    c.expiry = 123
    c.extra["foo"] = "bar"
    msg1 = enc.encodeCredentials (c)

    del c.expiry
    msg2 = enc.encodeCredentials (c)

    c.extra["bar"] =  "baz"
    msg3 = enc.encodeCredentials (c)

    c.name = "nm2"
    msg4 = enc.encodeCredentials (c)

    c.name = "name"
    c.app = "app2"
    msg4 = enc.encodeCredentials (c)

    c.app = "app"
    c.expiry = 123
    del c.extra["bar"]
    msg5 = enc.encodeCredentials (c)

    self.assertEqual (msg1.header, msg2.header)
    self.assertEqual (msg1.header, msg3.header)
    self.assertEqual (msg1.header, msg4.header)
    self.assertEqual (msg1.header, msg5.header)

    self.assertNotEqual (msg1.body, msg2.body)
    self.assertNotEqual (msg2.body, msg3.body)
    self.assertNotEqual (msg3.body, msg4.body)
    self.assertEqual (msg1.body, msg5.body)

  def testExtraOrder (self):
    enc = Encoder (137, self.ADDR1)

    c = Credentials ("name", "app")
    c.extra = {"foo": "bar", "abc": "baz"}
    msg1 = enc.encodeCredentials (c)

    c.extra = {"abc": "baz", "foo": "bar"}
    msg2 = enc.encodeCredentials (c)

    self.assertEqual (msg1.body, msg2.body)

################################################################################

class VerifierTest (XidTest):

  def register (self, name, owner=None):
    sender = self.env.contracts.account
    self.env.register ("p", name, addr=sender)
    self.generate (1)

    if owner is not None:
      tokenId = self.env.contracts.registry.functions \
          .tokenIdForName ("p", name).call ()
      self.env.contracts.registry.functions \
          .transferFrom (sender, owner, tokenId) \
          .transact ({"from": sender})
      self.generate (1)

  @staticmethod
  def createCredentials (name, app):
    cred = Credentials (name, app)
    cred.protocol = Protocol.DELEGATION_CONTRACT
    return cred

  def signWith (self, cred, signer):
    """
    Signs the given credentials (adds a raw_signature) with the
    given signer address.
    """

    msg = self.verifier.encodeCredentials (cred)
    acc = self.env.lookupSignerAccount (signer)
    signature = acc.sign_message (msg)
    cred.raw_signature = signature.signature

  def expectInvalid (self, cred):
    """
    Expects that the given credentials are invalid when verified.
    """

    try:
      self.verifier.verifyCredentials (cred)
      raise RuntimeError ("expected credentials to be invalid")
    except InvalidCredentialsError:
      pass

  def run (self):
    self.generate (1)

    abi = Verifier.loadAbi ("XayaDelegation")
    self.contracts.delegation = self.env.evm.deployContract (
        self.env.contracts.account, abi,
        self.contracts.registry.address, "0x" + "00" * 20)

    self.verifier = Verifier (self.env.evm.w3,
                              self.contracts.delegation.address)

    snapshot = self.env.snapshot ()

    self.testIsRegistered ()
    snapshot.restore ()

    self.testInvalidProtocol ()
    snapshot.restore ()

    self.testExpiry ()
    snapshot.restore ()

    self.testSignatureCommitment ()
    snapshot.restore ()

    self.testAddressPermissions ()
    snapshot.restore ()

  def testIsRegistered (self):
    self.register ("domob")
    self.register ("XY Z")
    self.assertEqual (self.verifier.isRegistered ("domob"), True)
    self.assertEqual (self.verifier.isRegistered ("andy"), False)
    self.assertEqual (self.verifier.isRegistered ("XY Z"), True)

  def testInvalidProtocol (self):
    addr = self.env.createSignerAddress ()
    self.register ("domob", addr)

    cred = self.createCredentials ("domob", "app")
    cred.protocol = Protocol.XID_GSP
    self.signWith (cred, addr)
    self.expectInvalid (cred)

  def testExpiry (self):
    addr = self.env.createSignerAddress ()
    self.register ("domob", addr)

    cred = self.createCredentials ("domob", "app")
    cred.expiry = 123
    self.signWith (cred, addr)
    self.expectInvalid (cred)

    cred.expiry = int (time.time ()) + 1_000
    self.signWith (cred, addr)
    self.verifier.verifyCredentials (cred)

    del cred.expiry
    self.signWith (cred, addr)
    self.verifier.verifyCredentials (cred)

  def testSignatureCommitment (self):
    addr = self.env.createSignerAddress ()
    self.register ("domob", addr)

    cred = self.createCredentials ("domob", "app")
    self.signWith (cred, addr)
    cred.name = "andy"
    self.expectInvalid (cred)

    # Even if the name exists the signature is invalid.
    self.register ("andy", addr)
    self.expectInvalid (cred)

    cred = self.createCredentials ("domob", "app")
    self.signWith (cred, addr)
    cred.app = "other app"
    self.expectInvalid (cred)

    cred = self.createCredentials ("domob", "app")
    self.signWith (cred, addr)
    cred.expiry = 12_345_678_900
    self.expectInvalid (cred)

    cred = self.createCredentials ("domob", "app")
    self.signWith (cred, addr)
    cred.extra["foo"] = "bar"
    self.expectInvalid (cred)

    # Re-sign to make it valid.
    self.signWith (cred, addr)
    self.verifier.verifyCredentials (cred)

  def testAddressPermissions (self):
    owner = self.env.createSignerAddress ()
    self.register ("domob", owner)

    # Make sure the owner address has some balance for gas.
    self.env.evm.w3.eth.send_transaction ({
      "from": self.env.contracts.account,
      "to": owner,
      "value": 10**18,
    })
    self.generate (1)

    tokenId = self.contracts.registry.functions \
        .tokenIdForName ("p", "domob").call ()
    noExpiry = 2**256 - 1

    approvedForAll = self.env.createSignerAddress ()
    approvedOne = self.env.createSignerAddress ()
    allPermission = self.env.createSignerAddress ()
    appPermission = self.env.createSignerAddress ()
    otherAppPermission = self.env.createSignerAddress ()
    calls = [
      self.contracts.registry.functions \
          .setApprovalForAll (approvedForAll, True),
      self.contracts.registry.functions.approve (approvedOne, tokenId),
      self.contracts.delegation.functions \
          .grant ("p", "domob", [], allPermission, noExpiry, False),
      self.contracts.delegation.functions \
          .grant ("p", "domob", ["g", "id", "xidauth", "app"],
                  appPermission, noExpiry, False),
      self.contracts.delegation.functions \
          .grant ("p", "domob", ["g", "id", "xidauth", "other app"],
                  otherAppPermission, noExpiry, False),
    ]
    nonce = self.env.evm.w3.eth.get_transaction_count (owner)
    for c in calls:
      tx = c.build_transaction ({
        "from": owner,
        "gas": 1_000_000,
        "nonce": nonce,
      })
      signed = self.env.lookupSignerAccount (owner).sign_transaction (tx)
      self.env.evm.w3.eth.send_raw_transaction (signed.raw_transaction)
      nonce += 1
    self.generate (1)

    for addr in [owner, approvedForAll, approvedOne,
                 allPermission, appPermission]:
      cred = self.createCredentials ("domob", "app")
      self.signWith (cred, addr)
      self.verifier.verifyCredentials (cred)

    cred = self.createCredentials ("domob", "app")
    self.signWith (cred, otherAppPermission)
    self.expectInvalid (cred)
    cred.app = "other app"
    self.signWith (cred, otherAppPermission)
    self.verifier.verifyCredentials (cred)

################################################################################

if __name__ == "__main__":
  unittest.main (argv=sys.argv[:1], exit=False)
  VerifierTest ().main ()
