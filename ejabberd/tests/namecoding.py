#!/usr/bin/env python
# coding=utf-8

# Copyright (C) 2019 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidauth import EjabberdXidAuth

import logging
import sys
import unittest


class NameCodingTest (unittest.TestCase):

  def setUp (self):
    self.auth = EjabberdXidAuth ({}, "http://localhost",
                                 logging.StreamHandler (sys.stderr))

  def testSimpleNames (self):
    simpleNames = ["domob", "0", "foo42bar", "xxx"]
    for n in simpleNames:
      self.assertEqual (self.auth.decodeXmppName (n), n)

  def testEncodedNames (self):
    names = {
      "x-": "",
      "x-782d666f6f": "x-foo",
      "x-c3a4c3b6c3bc": u"äöü",
      "x-466f6f20426172": "Foo Bar",
    }
    for enc, nm in names.iteritems ():
      self.assertEqual (self.auth.decodeXmppName (enc), nm)

  def testInvalidNames (self):
    invalid = [
      # Empty string is invalid (should be hex-encoded).
      "",

      # Invalid characters for simple names.
      "domob foobar", "Abc", "abc.def", "no-dash", "dom\nob", u"äöü",

      # Invalid hex characters (including upper case for otherwise valid name).
      "x-x", "x-2D", "x-\nabc",

      # Hex-encoded name that is actually simple.
      "x-616263",
    ]
    for n in invalid:
      self.assertEqual (self.auth.decodeXmppName (n), None)


if __name__ == "__main__":
  unittest.main ()
