#!/usr/bin/env python3

# Copyright (C) 2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidauth.credentials import Credentials, Protocol
from xidauth.delegation import Encoder

import binascii
import unittest


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


if __name__ == "__main__":
  unittest.main ()
