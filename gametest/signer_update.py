#!/usr/bin/env python3

# Copyright (C) 2019-2021 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidtest import XidTest

"""
Tests basic move processing for updating the signer addresses.
"""


class SignerUpdateTest (XidTest):

  def expectSigners (self, expected):
    """
    Retrieves the current game state and asserts that the signers for names
    match the ones given in the expected dictionary.
    """

    nmState = self.getGameState ()["names"]
    self.assertEqual (len (nmState), len (expected))

    for nm, s in nmState.items ():
      assert nm in expected
      self.assertEqual (s["signers"], expected[nm])

  def updateSigners (self, name, newGlobal, newApps):
    """
    Sends a move to update the signers for the given name.
    Both newGlobal and newApps may be None to not update global or
    app signers.
    """

    upd = {}

    if newGlobal is not None:
      upd["g"] = newGlobal
    if newApps is not None:
      upd["a"] = newApps

    self.sendMove (name, {"s": upd})

  def run (self):
    self.generate (101)
    self.expectSigners ({})

    addr = []
    for _ in range (4):
      addr.append (self.env.createSignerAddress ())

    # The game state returns addresses in ascending order in arrays (that's
    # not guaranteed by the specification but how it works currently).  Thus
    # we need to sort the addr array to ensure we can reproduce the exact
    # ordering in the comparison to our golden value.
    addr.sort ()

    self.updateSigners ("foo", [addr[0], addr[1]], None)
    self.updateSigners ("bar", None, {"a": [addr[2]], "b": [addr[3]]})
    self.generate (1)
    self.expectSigners ({
      "foo": [
        {"addresses": [addr[0], addr[1]]},
      ],
      "bar": [
        {
          "application": "a",
          "addresses": [addr[2]]
        },
        {
          "application": "b",
          "addresses": [addr[3]]
        },
      ],
    })

    self.updateSigners ("foo", [], None)
    self.updateSigners ("bar", None, {"a": []})
    self.generate (1)
    self.expectSigners ({
      "bar": [
        {
          "application": "b",
          "addresses": [addr[3]]
        },
      ],
    })


if __name__ == "__main__":
  SignerUpdateTest ().main ()
