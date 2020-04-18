# XID's RPC Interface

This document describes the [JSON format](#json) of the XID state
that is used in its RPC interface, as well as the available
[RPC methods](#rpc) themselves.

## <a id="json">JSON Format of the Game State</a>

For communication with the XID daemon, all data is encoded
in the [JSON format](https://json.org/).

### <a id="json-one-name">Data for Names</a>

The data for **individual names** is represented by a JSON object of the
following format:

    {
      "name": NAME,
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
        ],
      "addresses":
        {
          CRYPTO1: CRYPTOADDR1,
          CRYPTO2: CRYPTOADDR2,
          ...
        },
    }

In particular, the `signers` field holds information about registered
*signer addresses*.  They are grouped together into blocks by application.
*Global signers* are given in a JSON object that has no `application` key.
Application-specific signers are given in objects with the application specified
as a string (which may be `""`).
All `GLOBAL`n and `ADDR`n signer addresses are XAYA addresses encoded as
strings.

The `addresses` field holds crypto addresses that the user has associated
for other coins and tokens.  Each `CRYPTO`n is a string that identifies the
coin/token (e.g. `btc`), with the `CRYPTOADDR`n being the corresponding address
also as a string.  The individual keys and formats for addresses are not
defined (or interpreted) further by XID itself.

For convenience, the name itself is repeated as `NAME` in the JSON state.

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

When run, the XID daemon `xid` exposes a
**[JSON-RPC 2.0](https://www.jsonrpc.org/)** interface
over HTTP on a local port (the port number is specified in its invocation).

All methods accept arguments in the **keyword-form**.

### Standard Methods from `libxayagame`

Since XID is based on
[`libxayagame`](https://github.com/xaya/libxayagame),
it has the standard methods from its
[`GameRpcServer`](https://github.com/xaya/libxayagame/blob/master/xayagame/gamerpcserver.hpp).
Those methods can be used for very basic operations.

#### <a id="getnullstate">`getnullstate`</a>

This method returns information about the current state of the XID process
itself:

    {
      "gameid": "id",
      "chain": CHAIN,
      "state": STATE,
      "blockhash": BLOCK,
      "height": HEIGHT,
    }

Here, the placeholders have the following meanings:

- **`CHAIN`** defines on which chain (`main`, `test` or `regtest`) the
  XID daemon is running.
- **`STATE`** is the current syncing state of the XID daemon.  It is typically
  `catching-up` while the daemon is still syncing or `up-to-date` if it is
  synced to the latest block.
- **`BLOCK`** is the block hash to which the current state corresponds.
- **`HEIGHT`** is the block height of the current state.

This is useful as the cheapest possible way to query the XID process
about its health and state, without the need to extract any extra
game-state data.

#### <a id="getcurrentstate">`getcurrentstate`</a>

This method returns general state information of XID similar to
[`getnullstate`](#getnullstate), but also includes the
[full state](#json-full) in an extra `gamestate` field.

#### `waitforchange`

This method blocks until the state of the XID daemon changes (typically because
a new block has been processed).  It returns the *block hash* of the new best
block as string.

In exceptional situations, this method may also return JSON `null` instead,
if no new best block is known.  This happens if the connected XAYA Core daemon
does not even have blocks until the initial state of XID yet.

#### `stop`

This is a JSON-RPC *notification* and simply requests the XID daemon
to shut down cleanly.

### Data Retrieval

In addition to the generic [`getcurrentstate`](#getcurrentstate) method which
returns the full game state, XID also exposes more specific methods for
retrieving certain parts of the game state.  Where possible, these methods
should be used, as they allow more efficient access to the required data.

#### <a id="getnamestate">`getnamestate`</a>

This method retrieves the data (if any) for **one specific name**.  The name
in question has to be passed as a JSON string to the keyword argument `name`.
Returned is the [state data for this name](#json-one-name) in the `data` field
of a JSON object otherwise like [`getnullstate`](#getnullstate).

### Authentication Credentials

XID has special RPC methods supporting its use for
[user authentication](auth.md).  They are able to construct credentials,
sign them and verify whether or not given credentials are valid.

#### <a name="getauthmessage">`getauthmessage`</a>

This method constructs an [authentication message](auth.md#auth-message)
for given data.  It has to be passed `name` and `application` as strings,
as well as additional data for construction of the authentication message in
the `data` argument.  Its value has to be a JSON object of the following form:

    {
      "expiry": EXPIRY,
      "extra":
        {
          KEY1: VALUE1,
          KEY2: VALUE2,
          ...
        }
    }

`EXPIRY`, if set and not `null`, must be an integer that specifies the
desired expiration time of the credentials as UNIX timestamp.  If it is
left out or `null`, then the credentials will not expire.

`extra` can be a dictionary holding `KEY`i to `VALUE`i mappings for the
extra data that should be included in the credentials.

On success, `getauthmessage` returns a JSON object like this:

    {
      "authmessage": AUTH-MESSAGE,
      "password": PASSWORD
    }

Here, `AUTH-MESSAGE` is the constructed authentication message as a string.
`PASSWORD` is the encoded password that holds the expiration and extra data,
*but no signature yet* (and is thus not yet valid).
[`setauthsignature`](#setauthsignature) can be used to add the signature
in a second step.

#### <a name="setauthsignature">`setauthsignature`</a>

This method can be used to add in the signature for an already-constructed
password (e.g. coming from [`getauthmessage`](#getauthmessage)).

It expects two string arguments, `password` and `signature`.  It returns
the amended password as string.

#### `verifyauth`

This method verifies whether or not given credentials are valid.  It accepts
`name`, `application` and `password` as string arguments.

Since the validity depends on the current game state, it returns a JSON
object similar to [`getnullstate`](#getnullstate).  In addition,
it has the verification data in `data`.

This verification result is a JSON object itself, of the following form:

    {
      "valid": VALID,
      "state": STATE,
      "expiry": EXPIRY,
      "extra":
        {
          KEY1: VALUE1,
          KEY2: VALUE2,
          ...
        }
    }

`VALID` is a boolean indicating whether or not the credentials are valid.  It is
only true if everything is fine and the credentials should be accepted by
the client application (subject to additional, application-specific checks).

The encoded expiry is returned in `EXPIRY` as a UNIX timestamp or `null` if
the credentials do not expire.  Any extra data present in the credentials is
returned in the `extra` dictionary.

`STATE`, finally, is a string giving more details about the validation and
the reason why credentials may not be valid.  It can hold any of the following
values:

- **`malformed`** indicates that the password string could not be decoded
  into an `AuthData` protocol buffer.
- **`invalid-data`** means that the protocol buffer or other fields (e.g.
  application name) have an invalid format.
- **`invalid-signature`** means that the signature was invalid or could not
  be tied to a signer key of the name and application.
- **`expired`** means that the credentials are valid but expired at the
  current system time.
- **`valid`** is returned if and only if `valid` is set to `true`.

#### `authwithwallet`

This method constructs *and signs* authentication credentials using
the wallet of the attached Xaya Core daemon.  To use it, XID has to
be started with `--allow_wallet`.

The `name`, `application` and `data` have to be passed as arguments.
They are defined as for [`getauthmessage`](#getauthmessage).

Returned is a JSON object similar to [`getnullstate`](#getnullstate).
In its `data` field, the constructed and fully signed password string
is returned.
