#!/usr/bin/env python

# Copyright (C) 2019 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from xidtest import XidTest

"""
Tests the getnamestate RPC method.
"""


class GetNameStateTest (XidTest):

  def run (self):
    self.generate (101)

    addr = self.rpc.xaya.getnewaddress ("", "legacy")
    self.sendMove ("domob", {"s": {"g": [addr]}})
    self.generate (1)

    self.assertEqual (self.getRpc ("getnamestate", name="domob"), {
      "signers":
        [
          {"addresses": [addr]},
        ],
      "addresses": {},
    })
    self.assertEqual (self.getRpc ("getnamestate", name="foo"), {
      "signers": [],
      "addresses": {},
    })


if __name__ == "__main__":
  GetNameStateTest ().main ()
