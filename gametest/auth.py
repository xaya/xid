#!/usr/bin/env python3
# coding=utf8

# Copyright (C) 2019-2022 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidtest import XidTest

import base64

"""
Tests the authentication RPC methods.
"""


class AuthTest (XidTest):

  def createPassword (self, name, app, addr, expiry, extra):
    """
    Creates a Xid password for the given data, signed with the given
    Xaya address.
    """

    d = {
      "expiry": expiry,
      "extra": extra,
    }
    authMsg = self.rpc.game.getauthmessage (name=name, application=app, data=d)

    signed = self.env.signMessage (addr, authMsg["authmessage"])
    pwd = self.rpc.game.setauthsignature (password=authMsg["password"],
                                          signature=signed)

    return pwd

  def run (self):
    self.generate (101)

    self.addrGeneral = self.env.createSignerAddress ()
    self.addrApp = self.env.createSignerAddress ()
    self.addrEmpty = self.env.createSignerAddress ()
    self.sendMove ("domob", {"s": {
      "g": ["invalid just for fun", self.addrGeneral],
      "a":
        {
          "": [self.addrEmpty],
          "app": [self.addrApp],
        },
    }})
    self.sendMove ("", {"s": {"g": [self.addrGeneral]}})
    self.generate (1)
    self.assertEqual (self.getRpc ("getnamestate", name="domob")["signers"], [
      {"addresses": [self.addrGeneral, "invalid just for fun"]},
      {
        "application": "",
        "addresses": [self.addrEmpty],
      },
      {
        "application": "app",
        "addresses": [self.addrApp],
      },
    ])
    self.assertEqual (self.getRpc ("getnamestate", name="")["signers"], [
      {"addresses": [self.addrGeneral]},
    ])

    self.testGetAuthMessage ()
    self.testAuthDataErrors ()
    self.testAuthDataValidation ()
    self.testPasswordErrors ()
    self.testVerification ()

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

  def testPasswordErrors (self):
    self.mainLogger.info ("Testing error handling for password strings...")

    self.expectError (2, "failed to parse the password string",
                      self.rpc.game.setauthsignature,
                      password="invalid base64", signature="")
    self.expectError (4, "the signature is not base64",
                      self.rpc.game.setauthsignature,
                      password="", signature="invalid base64")

  def testVerification (self):
    self.mainLogger.info ("Testing credentials verification...")

    expired = 42
    notExpired = 2000000000

    # Invalid encoded data.
    for pwd in ["invalid base64",
                base64.b64encode (bytes ([0, 20])).decode ("ascii")]:
      res = self.getRpc ("verifyauth", name="domob", application="app",
                         password=pwd)
      self.assertEqual (res, {
        "valid": False,
        "state": "malformed",
      })

    # Invalid application string.
    res = self.getRpc ("verifyauth", name="domob", application="invalid app",
                       password="")
    self.assertEqual (res, {
      "valid": False,
      "state": "invalid-data",
    })

    # Invalid signature (unauthorised key).  This is also expired, to make
    # sure that we get an "invalid signature" error rather than "expired"
    # for this case.
    pwd = self.createPassword ("domob", "other", self.addrApp, expired, {})
    res = self.getRpc ("verifyauth", name="domob", application="other",
                       password=pwd)
    self.assertEqual (res, {
      "valid": False,
      "state": "invalid-signature",
      "expiry": expired,
      "extra": {},
    })

    # Invalid signature (changed name).
    pwd = self.createPassword ("domob", "app", self.addrGeneral, None, {})
    res = self.getRpc ("verifyauth", name="other", application="app",
                       password=pwd)
    self.assertEqual (res, {
      "valid": False,
      "state": "invalid-signature",
      "expiry": None,
      "extra": {},
    })

    # Invalid signature (changed application).
    pwd = self.createPassword ("domob", "app", self.addrGeneral, None, {})
    res = self.getRpc ("verifyauth", name="domob", application="other",
                       password=pwd)
    self.assertEqual (res, {
      "valid": False,
      "state": "invalid-signature",
      "expiry": None,
      "extra": {},
    })

    # Expired but otherwise valid.
    pwd = self.createPassword ("domob", "app", self.addrGeneral, expired, {})
    res = self.getRpc ("verifyauth", name="domob", application="app",
                       password=pwd)
    self.assertEqual (res, {
      "valid": False,
      "state": "expired",
      "expiry": expired,
      "extra": {},
    })

    # Valid without expiry and signed with general key.
    extra = {
      "some": "stuff",
      "for": "testing",
    }
    pwd = self.createPassword ("domob", "app", self.addrGeneral, None, extra)
    res = self.getRpc ("verifyauth", name="domob", application="app",
                       password=pwd)
    self.assertEqual (res, {
      "valid": True,
      "state": "valid",
      "expiry": None,
      "extra": extra,
    })

    # Valid without expiry signed with app-specific key.
    pwd = self.createPassword ("domob", "app", self.addrApp, None, {})
    res = self.getRpc ("verifyauth", name="domob", application="app",
                       password=pwd)
    self.assertEqual (res, {
      "valid": True,
      "state": "valid",
      "expiry": None,
      "extra": {},
    })

    # Valid with expiry.
    pwd = self.createPassword ("domob", "app", self.addrGeneral, notExpired, {})
    res = self.getRpc ("verifyauth", name="domob", application="app",
                       password=pwd)
    self.assertEqual (res, {
      "valid": True,
      "state": "valid",
      "expiry": notExpired,
      "extra": {},
    })

    # Empty application name does not work as general signer.
    pwd = self.createPassword ("domob", "app", self.addrEmpty, notExpired, {})
    res = self.getRpc ("verifyauth", name="domob", application="app",
                       password=pwd)
    self.assertEqual (res, {
      "valid": False,
      "state": "invalid-signature",
      "expiry": notExpired,
      "extra": {},
    })

    # Empty application name is fine with app-specific signer.
    pwd = self.createPassword ("domob", "", self.addrEmpty, notExpired, {})
    res = self.getRpc ("verifyauth", name="domob", application="",
                       password=pwd)
    self.assertEqual (res, {
      "valid": True,
      "state": "valid",
      "expiry": notExpired,
      "extra": {},
    })

    # Empty name is also fine.
    pwd = self.createPassword ("", "app", self.addrGeneral, notExpired, {})
    res = self.getRpc ("verifyauth", name="", application="app", password=pwd)
    self.assertEqual (res, {
      "valid": True,
      "state": "valid",
      "expiry": notExpired,
      "extra": {},
    })


if __name__ == "__main__":
  AuthTest ().main ()
