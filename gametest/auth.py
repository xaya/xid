#!/usr/bin/env python
# coding=utf8

# Copyright (C) 2019 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidtest import XidTest

"""
Tests the authentication RPC methods.
"""


class AuthTest (XidTest):

  def run (self):
    self.generate (101)

    addrGeneral = self.rpc.xaya.getnewaddress ("", "legacy")
    addrApp = self.rpc.xaya.getnewaddress ("", "legacy")
    self.sendMove ("domob", {"s": {
      "g": [addrGeneral],
      "a":
        {
          "app": [addrApp],
        },
    }})
    self.generate (1)
    self.assertEqual (self.getRpc ("getnamestate", name="domob"), {
      "signers":
        [
          {"addresses": [addrGeneral]},
          {
            "application": "app",
            "addresses": [addrApp],
          },
        ],
    })

    self.testGetAuthMessage ()
    self.testAuthDataErrors ()
    self.testAuthDataValidation ()

  def testGetAuthMessage (self):
    self.mainLogger.info ("Testing getauthmessage...")

    res = self.rpc.game.getauthmessage (name="äöü", application="app", data={
      "expiry": None,
      "extra":
        {
          "foo": "bar",
          "abc": "def",
        },
    })
    self.assertEqual (res["authmessage"],
                      u"Xid login\n"
                      u"äöü\n"
                      u"at: app\n"
                      u"expires: never\n"
                      u"extra:\n"
                      u"abc=def\n"
                      u"foo=bar\n")
    assert "password" in res

    res = self.rpc.game.getauthmessage (name="domob", application="app", data={
      "expiry": 1234,
    })
    self.assertEqual (res["authmessage"],
                      "Xid login\n"
                      "domob\n"
                      "at: app\n"
                      "expires: 1234\n"
                      "extra:\n")
    assert "password" in res

  def testAuthDataErrors (self):
    self.mainLogger.info ("Testing error checking for the auth data...")

    tests = [
      {"code": -32602, "msg": ".*Invalid method parameters.*", "data": 42},
      {
        "code": -1,
        "msg": "expiry must be an integer",
        "data": {"expiry": "foo"},
      },
      {
        "code": -1,
        "msg": "extra must be an object",
        "data": {"extra": 5},
      },
      {
        "code": -1,
        "msg": "extra value must be a string",
        "data": {"extra": {"foo": False}},
      },
    ]
    for t in tests:
      self.expectError (t["code"], t["msg"], self.rpc.game.getauthmessage,
                        name="domob", application="app", data=t["data"])

  def testAuthDataValidation (self):
    self.mainLogger.info ("Testing validation of auth data...")

    tests = [
      {"name": "do\nmob", "application": "app", "data": {}},
      {"name": "domob", "application": "foo bar", "data": {}},
      {"name": "domob", "application": "app", "data": {"extra": {"a b": "c"}}},
      {"name": "domob", "application": "app", "data": {"extra": {"a": "b c"}}},
    ]

    for t in tests:
      self.expectError (1, "the authentication data is invalid",
                        self.rpc.game.getauthmessage,
                        name=t["name"], application=t["application"],
                        data=t["data"])


if __name__ == "__main__":
  AuthTest ().main ()
