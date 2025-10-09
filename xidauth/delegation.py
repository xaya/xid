# Copyright (C) 2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from . import credentials

from eth_account.messages import encode_typed_data
from web3 import Web3

import json
import os


class Encoder:
  """
  This class implements the encoding of data from Credentials according
  to the EIP-712 format used for signing and verification.
  """

  def __init__ (self, chainId, contract):
    if not Web3.is_address (contract):
      raise RuntimeError (f"invalid contract address: {contract}")

    self.chainId = chainId
    self.contract = Web3.to_checksum_address (contract)

  def encodeCredentials (self, cred):
    """
    Encodes all data from the given Credentials instance into the
    EIP-712 typed-data hash corresponding to it (which is what should be
    signed or verified).  Returns a SignableMessage.
    """

    msg = {
      "name": cred.name,
      "application": cred.app,
      "expiry": -1 if (cred.expiry is None) else cred.expiry,
      "extra": [
        {"key": k, "value": v}
        for (k, v) in sorted (cred.extra.items ())
      ],
    }

    full = {
      "domain": {
        "name": "xidauth delegation-contract",
        "version": "1",
        "chainId": self.chainId,
        "verifyingContract": self.contract,
      },
      "primaryType": "XidAuthChallenge",
      "types": {
        "XidAuthChallenge": [
          {"name": "name", "type": "string"},
          {"name": "application", "type": "string"},
          {"name": "expiry", "type": "int64"},
          {"name": "extra", "type": "ExtraData[]"},
        ],
        "ExtraData": [
          {"name": "key", "type": "string"},
          {"name": "value", "type": "string"},
        ],
      },
      "message": msg,
    }

    return encode_typed_data (full_message=full)
