#!/usr/bin/env python3

# Copyright (C) 2019-2020 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidtest import XidTest

"""
Tests enabling / disabling the Xaya Core wallet with --allow_wallet.
"""


class AllowWalletTest (XidTest):

  def setWalletEnabled (self, enabled):
    self.log.info ("Enabling wallet: %d" % enabled)

    extraArgs = []
    if enabled:
      extraArgs.append ("--allow_wallet")

    self.stopGameDaemon ()
    self.startGameDaemon (extraArgs)

  def run (self):
    self.generate (101)

    # Register signer keys for testing authwithwallet.
    addr = self.rpc.xaya.getnewaddress ("", "legacy")
    self.sendMove ("domob", {"s": {"g": [addr]}})
    self.generate (1)

    # Expect errors for disabled wallet.
    self.setWalletEnabled (False)
    self.expectError (-2, ".*Xaya wallet is not enabled.*",
                      self.rpc.game.authwithwallet,
                      name="domob", application="app", data={})

    # All should be fine with enabled wallet.
    self.setWalletEnabled (True)
    self.getRpc ("authwithwallet", name="domob", application="app", data={})


if __name__ == "__main__":
  AllowWalletTest ().main ()
