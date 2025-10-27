# Copyright (C) 2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from . import credentials

from eth_account.messages import encode_typed_data
from web3 import Web3

import json
import logging
import os
import time


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


class Verifier:
  """
  This class implements credential verification for xidauth with
  delegation-contract, talking to the on-chain smart contracts and
  verifying EIP-712 signatures.
  """

  def __init__ (self, w3, delegationAddr):
    self.log = logging.getLogger ("xidauth.delegation")

    self.w3 = w3
    self.chainId = self.w3.eth.chain_id
    self.log.info (f"Connected to chain ID: {self.chainId}")

    abi = self.loadAbi ("XayaDelegation")
    self.delegation = self.w3.eth.contract (address=delegationAddr,
                                            abi=abi["abi"])

    accountsAddr = self.delegation.functions.accounts ().call ()
    abi = self.loadAbi ("XayaAccounts")
    self.accounts = self.w3.eth.contract (address=accountsAddr, abi=abi["abi"])

    self.log.info (f"Accounts contract: {self.accounts.address}")
    self.log.info (f"Delegation contract: {self.delegation.address}")

    self.encoder = Encoder (self.w3.eth.chain_id, self.delegation.address)

  @staticmethod
  def loadAbi (nm):
    """
    Loads an ABI file from the abi/ folder relative to the current
    Python file.
    """

    thisDir = os.path.dirname (os.path.abspath (__file__))
    abiFile = os.path.join (thisDir, "abi", f"{nm}.json")

    with open (abiFile, "rt") as f:
      return json.load (f)

  def isRegistered (self, name):
    """
    Checks whether or not the given Xaya account name is registered
    at all.  Returns true if it is, and false if not.
    """

    return self.accounts.functions.exists ("p", name).call ()

  def encodeCredentials (self, cred):
    return self.encoder.encodeCredentials (cred)

  def verifyCredentials (self, cred):
    """
    Verifies a given Credentials instance.  Throws InvalidCredentialsError
    if it is invalid, and silently succeeds if it is valid.
    """

    if cred.protocol != credentials.Protocol.DELEGATION_CONTRACT:
      raise credentials.InvalidCredentialsError (
          f"unsupported signing protocol (only delegation-contract): {cred.protocol}")

    if cred.expired:
      raise credentials.InvalidCredentialsError ("credentials are expired")

    tokenId = self.accounts.functions.tokenIdForName ("p", cred.name).call ()
    if not self.accounts.functions.exists (tokenId).call ():
      raise credentials.InvalidCredentialsError (
          f"name '{cred.name}' does not exist")

    msg = self.encodeCredentials (cred)
    signer = self.w3.eth.account.recover_message (
                msg, signature=cred.raw_signature)

    # We want the owner, all ERC721-approved addresses, and anyone with
    # delegation permissions on ["g", "id", "xidauth", application] to
    # have access.  The delegation contract handles owner (has always access)
    # and the delegation part, but we need to explicitly check for ERC721
    # approvals in addition.

    path = ["g", "id", "xidauth", cred.app]
    if self.delegation.functions.hasAccess (
        "p", cred.name, path, signer, int (time.time ())).call ():
      return

    if self.accounts.functions.getApproved (tokenId).call () == signer:
      return

    owner = self.accounts.functions.ownerOf (tokenId).call ()
    if self.accounts.functions.isApprovedForAll (owner, signer).call ():
      return

    raise credentials.InvalidCredentialsError (
        f"signer address {signer} does not have permission for name '{cred.name}'")
