#!/usr/bin/env python3

# Copyright (C) 2019-2020 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidtest import XidTest

"""
Tests basic move processing for updating the crypto-address associations.
"""


class AddressUpdateTest (XidTest):

  def expectAddresses (self, expected):
    """
    Retrieves the current game state and asserts that the addresses
    associated to names match the ones given in the expected dictionary.
    """

    nmState = self.getGameState ()["names"]
    self.assertEqual (len (nmState), len (expected))

    for nm, s in nmState.items ():
      assert nm in expected
      self.assertEqual (s["addresses"], expected[nm])

  def run (self):
    self.generate (101)
    self.expectAddresses ({})

    self.sendMove ("foo", {
      "ca": {"btc": "1foo", "eth": "0xfoo"},
    })
    self.sendMove ("bar", {
      "ca": {"btc": None, "chi": "Cabcdef"},
    })
    self.generate (1)
    self.expectAddresses ({
      "foo":
        {
          "btc": "1foo",
          "eth":  "0xfoo",
        },
      "bar":
        {
          "chi": "Cabcdef",
        },
    })

    self.sendMove ("foo", {
      "ca": {"eth": None},
    })
    self.sendMove ("bar", {
      "ca": {"chi": None},
    })
    self.generate (1)
    self.expectAddresses ({
      "foo":
        {
          "btc": "1foo",
        },
    })


if __name__ == "__main__":
  AddressUpdateTest ().main ()
