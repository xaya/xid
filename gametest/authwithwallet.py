#!/usr/bin/env python

# Copyright (C) 2019 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidtest import XidTest

import base64

"""
Tests constructing auth credentials signed by the Xaya wallet directly
(i.e. the authwithwallet RPC method).
"""

WALLET_PWD = "wallet passphrase"


class AuthWithWalletTest (XidTest):

  def setWalletLocked (self, locked):
    """
    Locks or unlocks the Xaya Core wallet.
    """

    self.log.info ("Setting wallet locked: %d" % locked)

    self.rpc.xaya.walletlock ()
    if not locked:
      self.rpc.xaya.walletpassphrase (WALLET_PWD, 60)

    info = self.rpc.xaya.getwalletinfo ()
    if locked:
      self.assertEqual (info["unlocked_until"], 0)
    else:
      assert info["unlocked_until"] > 0

  def signAndCheck (self, name, application):
    """
    Tries to construct credentials for the given name and application
    and then verify they are valid.
    """

    expiry = 2000000000
    extra = {
      "foo": "bar",
      "abc": "def",
    }
    data = {
      "expiry": expiry,
      "extra": extra,
    }

    expiry = None
    if "expiry" in data:
      expiry = data["expiry"]
    extra = {}
    if "extra" in data:
      extra = data["extra"]

    pwd = self.getRpc ("authwithwallet", name=name, application=application,
                       data=data)
    verify = self.getRpc ("verifyauth", name=name, application=application,
                          password=pwd)
    self.assertEqual (verify, {
      "valid": True,
      "state": "valid",
      "expiry": expiry,
      "extra": extra,
    })

  def run (self):
    self.generate (101)
    self.stopGameDaemon ()
    self.startGameDaemon (["--allow_wallet"])

    addrNotInWallet = "cfFTXqKsRRVXYNKjWvHkipop2E5JvoKoSY"
    info = self.rpc.xaya.getaddressinfo (addrNotInWallet)
    self.assertEqual (info["ismine"], False)

    self.mainLogger.info ("Encrypting wallet of the test instance...")
    self.rpc.xaya.encryptwallet (WALLET_PWD)
    info = self.rpc.xaya.getwalletinfo ()
    self.assertEqual (info["unlocked_until"], 0)
    self.setWalletLocked (False)

    self.mainLogger.info ("Invalid address as key...")
    self.sendMove ("domob", {"s": {"g": ["invalid address", addrNotInWallet]}})
    self.generate (1)
    self.expectError (3, "no authorised key is in the wallet",
                      self.signAndCheck, "domob", "app")

    self.mainLogger.info ("Signing with application-specific key...")
    addr = self.rpc.xaya.getnewaddress ("", "legacy")
    self.sendMove ("domob", {"s": {"a": {"app": [addr]}}})
    self.generate (1)
    self.signAndCheck ("domob", "app")

    self.mainLogger.info ("Testing error for no valid key...")
    self.expectError (3, "no authorised key is in the wallet",
                      self.signAndCheck, "domob", "other")

    self.mainLogger.info ("Testing locked wallet...")
    self.setWalletLocked (True)
    self.expectError (-3, "the Xaya wallet is locked",
                      self.signAndCheck, "domob", "app")
    self.setWalletLocked (False)

    self.mainLogger.info ("Signing with global key...")
    self.sendMove ("domob", {"s": {"g": [addr]}})
    self.generate (1)
    self.signAndCheck ("domob", "other")


if __name__ == "__main__":
  AuthWithWalletTest ().main ()
