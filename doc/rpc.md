# Xid's RPC Interface

This document describes the [JSON format](#json) of the Xid state
that is used in its RPC interface, as well as the available
[RPC methods](#rpc) themselves.

## <a id="json">JSON Format of the Game State</a>

For communication with the Xid daemon, all data is encoded
in the [JSON format](https://json.org/).

### <a id="json-one-name">Data for Names</a>

The data for **individual names** is represented by a JSON object of the
following format:

    {
      "signers":
        [
          {
            "addresses": [GLOBAL1, GLOBAL2, ...]
          },
          {
            "application": APPLICATION,
            "addresses": [ADDR1, ADDR2, ...]
          },
          ...
        ]
    }

In particular, the `signers` field holds information about registered
*signer addresses*.  They are grouped together into blocks by application.
*Global signers* are given in a JSON object that has no `application` key.
Application-specific signers are given in objects with the application specified
as a string (which may be `""`).
All `GLOBAL`n and `ADDR`n signer addresses are XAYA addresses encoded as
strings.

### <a id="json-full">Full Game State</a>

The full game state can also be encoded as JSON.  This data can be very large,
though, so that client applications are encouraged to not rely on it except
for debugging and testing.

The **full game state** is a JSON object of the following format:

    {
      "names":
        {
          NAME1: DATA1,
          NAME2: DATA2,
          ...
        }
    }

The keys into `names` are the XAYA names for which non-trivial data is present.
The corresponding `DATA`n values are JSON objects with the
[data for those names](#json-one-name).

## <a id="rpc">RPC Methods</a>

When run, the Xid daemon `xid` exposes a
**[JSON-RPC 2.0](https://www.jsonrpc.org/)** interface
over HTTP on a local port (the port number is specified in its invocation).

### Standard Methods from `libxayagame`

Since Xid is based on
[`libxayagame`](https://github.com/xaya/libxayagame),
it has the standard methods from its
[`GameRpcServer`](https://github.com/xaya/libxayagame/blob/master/xayagame/gamerpcserver.hpp).
Those methods can be used for very basic operations.

#### <a id="getcurrentstate">`getcurrentstate`</a>

This method retrieves information about the current state of Xid.  This
includes the [full state](#json-full) as well as general information about the
state of the Xid daemon (e.g. how far it is synced to the blockchain).

The returned data is a JSON object of the following form:

    {
      "gameid": "id",
      "chain": CHAIN,
      "state": STATE,
      "blockhash": BLOCK,
      "height": HEIGHT,
      "gamestate": STATE
    }

Here, the placeholders have the following meanings:

- **`CHAIN`** defines on which chain (`main`, `test` or `regtest`) the
  Xid daemon is running.
- **`STATE`** is the current syncing state of the Xid daemon.  It is typically
  `catching-up` while the daemon is still syncing or `up-to-date` if it is
  synced to the latest block.
- **`BLOCK`** is the block hash to which the current state corresponds.
- **`HEIGHT`** is the block height of the current state.
- **`STATE`** is the actual [game state as JSON object](#json-full).

#### `waitforchange`

This method blocks until the state of the Xid daemon changes (typically because
a new block has been processed).  It returns the *block hash* of the new best
block as string.

In exceptional situations, this method may also return JSON `null` instead,
if no new best block is known.  This happens if the connected XAYA Core daemon
does not even have blocks until the initial state of Xid yet.

#### `stop`

This is a JSON-RPC *notification* and simply requests the Xid daemon
to shut down cleanly.

### Data Retrieval

In addition to the generic [`getcurrentstate`](#getcurrentstate) method which
returns the full game state, Xid also exposes more specific methods for
retrieving certain parts of the game state.  Where possible, these methods
should be used, as they allow more efficient access to the required data.

#### `getnamestate`

This method retrieves the data (if any) for **one specific name**.  The name
in question has to be passed as a JSON string to the keyword argument `name`.
Returned is the [state data for this name](#json-one-name) as JSON object.

### Verification Methods

### Wallet Methods
