#!/usr/bin/env python3

# Copyright (C) 2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidauth.credentials import Credentials, Protocol

import time
import unittest


class CredentialsTest (unittest.TestCase):

  def setUp (self):
    self.cred = Credentials ("name", "app")

  def assertExtraEqual (self, cred, expected):
    """
    Helper method to assert the extra data (which is like a dict
    but not exactly a dict) matches the expected data.
    """

    self.assertSetEqual (set (cred.extra.items ()), set (expected.items ()))

  def testExpiry (self):
    self.assertEqual (self.cred.expiry, None)
    self.assertFalse (self.cred.expired)

    self.cred.expiry = 123
    self.assertEqual (self.cred.expiry, 123)
    self.assertTrue (self.cred.expired)

    self.cred.expiry = int (time.time () + 1_000)
    self.assertFalse (self.cred.expired)

    del self.cred.expiry
    self.assertEqual (self.cred.expiry, None)

  def testProtocol (self):
    self.assertEqual (self.cred.protocol, Protocol.XID_GSP)

    self.cred.protocol = Protocol.DELEGATION_CONTRACT
    self.assertEqual (self.cred.protocol, Protocol.DELEGATION_CONTRACT)

    del self.cred.protocol
    self.assertEqual (self.cred.protocol, Protocol.XID_GSP)

  def testExtra (self):
    self.assertExtraEqual (self.cred, {})

    self.cred.extra["foo"] = "bar"
    self.cred.extra["baz"] = "abc"
    self.assertExtraEqual (self.cred, {"foo": "bar", "baz": "abc"})

    self.cred.extra = {"x": "y", "z": "q"}
    self.assertExtraEqual (self.cred, {"x": "y", "z": "q"})

    del self.cred.extra
    self.assertExtraEqual (self.cred, {})

  def testSignature (self):
    self.cred.raw_signature = b"foobar"
    self.assertEqual (self.cred.raw_signature, b"foobar")

  def testPassword (self):
    self.cred.expiry = 123
    self.cred.protocol = Protocol.DELEGATION_CONTRACT
    self.cred.extra["foo"] = "bar"
    self.cred.raw_signature = b"foobar"

    c2 = Credentials ("name2", "app2")
    c2.password = self.cred.password

    self.assertEqual (c2.name, "name2")
    self.assertEqual (c2.app, "app2")
    self.assertEqual (c2.expiry, 123)
    self.assertEqual (c2.protocol, Protocol.DELEGATION_CONTRACT)
    self.assertExtraEqual (c2, {"foo": "bar"})


if __name__ == "__main__":
  unittest.main ()
