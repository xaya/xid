# XAYA ID

XAYA ID (*Xid*) is an application built on the XAYA platform, that
turns each XAYA name into a **secure digital identity** similar to
[NameID](https://nameid.org/).

These identities are meant to be used inside the XAYA ecosystem,
e.g. on chat systems or market places in XAYA games.  They can, however, be
used by any other application as well.  For instance, websites can enable
"login with XAYA" to use secure, password-less authentication.  Or messaging
systems can use XAYA identities for the secure exchange of key fingerprints
for end-to-end encryption.

## Overview

From a high-level point of view, Xid allows owners of XAYA names to
**associate metadata** to their names, that **everyone else can read**.
Due to how XAYA works, **only the owners of names** are able to modify
the data (through the keys in their wallets).  This ensures that the data
can be trusted by everyone.

There are two main types of data that can be associated with names:

- **Signer addresses** can be used to enable authentication within applications.
  These are ordinary XAYA addresses from the user's wallet; to authenticate with
  an application, their associated private key is used to sign a specific
  message.  A name can have multiple registered signers, for instance for
  XAYA wallets on different devices.  Certain addresses can be either *global*
  signers (valid generally for all applications), or they can be set
  specifically for certain applications.

- **Fingerprints of encryption keys** can be associated to names as well.
  This allows the secure and trusted exchange of keys, e.g. for signed emails
  ([GPG](https://gnupg.org/)) or messaging ([OTR](https://otr.cypherpunks.ca/)).
  *This is not yet implemented in Xid, but will come in the future.*

## Technical Details

More details can be found in specific documents:

- [Xid game](doc/game.md): Details about the game state and move format
  for Xid as game on the XAYA platform.
- [RPC interface](doc/rpc.md): The JSON format for game states and the RPC
  interface of the Xid daemon.
