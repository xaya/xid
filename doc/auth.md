# Signer Authentication

In this document, we describe the protocol for **authentication with a Xid
signer address** in detail.  This protocol enables users to prove that they
control a signer key for some XAYA name, so that services that use Xid
for user authentication can allow them to log in or perform other actions
on behalf of the XAYA name.

The authentication protocol described here is implemented directly in Xid,
which exposes [RPC methods](rpc.md) for verifying credentials and
helpers for constructing them for user signatures.  This makes it very easy to
support Xid authentication in third-party projects.

## Overview

Authentication for Xid works through a **username** and a **password**.
This makes it easy to fit into the typical workflow and existing
applications.  The username is simply the XAYA name (without the `p/`
namespace prefix).  The password is a [special data structure](#password) which
contains some metadata and, mainly, a *message
[signed](#signature) with one of the name's signer addresses*.

This *authentication message* is [constructed deterministically](#auth-message)
from the username, application name and metadata.  By signing it, the user
proves control over one of the signer keys associated to the name that
is being authenticated.

## <a id="password">Structure of the Password</a>

The password is a serlialised
[**protocol buffer**](https://developers.google.com/protocol-buffers/), encoded
in [OpenSSL's
**Base64**](https://www.openssl.org/docs/manmaster/man3/EVP_EncodeBlock.html)
format (with all new lines / whitespace removed).
In particular, it is an instance of the `AuthData` message as specified
in [`auth.proto`](https://github.com/xaya/xid/blob/master/auth/auth.proto).

The main content and only mandatory field in the protocol buffer is the
signature of the authentication message in the `signature_bytes` field.
This field holds the data corresponding to the signature returned by
Xaya Core's `signmessage` command, with the Base64 string decoded into bytes.

Other fields may be present as needed by a specific application; if they are,
then they influence the
[construction of the authentication message](#auth-message) itself:

- `expiry` can be set to the Unix timestamp at which the credentials expire.
  If this field is set, then the resulting password is only valid up to this
  time.  This allows construction of "one-time passwords".
- `extra` can hold arbitrary key/value pairs that can encode further,
   application-specific metadata.  Keys and values must be strings that
   only contain `.` and alphanumeric characters.
   Complex data can be encoded as hex if needed.
   Keys must be unique.

## <a id="auth-message">Construction of the Authentication Message</a>

The *authentication message* is constructed deterministically based on the
username, the application's name and the
[data encoded in the password](#password).  It is a string of the following
form:

    Xid login
    NAME
    at: APPLICATION
    expires: EXPIRY
    extra:
    KEY1=VALUE1
    KEY2=VALUE2
    ...

The placeholders have the following meaning:

- **`NAME`** is the XAYA name (encoded as UTF-8) for which authentication
  is performed.  This matches the username of the authentication credentials.
  Note that XAYA names are not allowed to contain new-line characters.
- **`APPLICATION`** is the name of the application for which the credentials
  are constructed.  It correponds to the string for which application-specific
  signer keys are assigned.  To be valid, the application name must only
  consist of alphanumeric characters, `.` and `/`.
- **`EXPIRY`** is either the string `never` if the credentials do not expire
  or the Unix timestamp (as a decimal numer) of the latest time at which
  they are valid.
- **`KEY`i** and **`VALUE`i** are the key/value pairs of extra metadata
  present in the password data structure.  They are sorted in ascending
  lexicographic order by the keys.

Each line ends with a single new-line character, including the last one.
In other words, the message either ends with `extra:` followed by a new line
if no extra data is present, or with `KEYn=VALUEn` followed by a new line
if there are `n` key/value pairs of extra data.

## <a id="signature">Message Signing</a>

To prove control over a signer key associated to the username
and application, the [authentication message](#auth-message) is
**signed using Xaya Core's `signmessage` functionality** with one of the
signer addresses.  This corresponds to an ECDSA signature with the
private key associated to the address, performed in a specific way and
encoded in a particular format.

## Full Verification Procedure

To verify credentials given by a username and password for a specific
application, these steps have to be performed:

1. The username must be a valid XAYA name and the application name must
   be valid (only alphanumeric characters, `.` and `/`).
2. It must be possible to [decode the password](#password) into a
   protocol buffer instance.
3. All fields in the resulting data structure must be valid as described
   above.
4. If an `expiry` timestamp is given, it must not be earlier than the
   current time.
5. The authentication message must be [constructed](#auth-message) from the
   username, application name and password data.
6. The `signature` given in the password must be a valid signature for
   the constructed authentication message according to Xaya Core's
   `verifymessage`.
7. The ECDSA public key
   [recovered from the signature](https://bitcoin.stackexchange.com/questions/60972/recovering-ecdsa-public-key-from-the-signature)
   must correspond, in legacy P2PKH format, either to a global signer address
   of the username or to a signer address for the username and application.

In addition to these steps, applications may have more requirements that
they check.  For instance, the application could give a connection-specific
nonce to the client who tries to authenticate.  Then it may require that
this nonce is included as one of the extra data fields in the password.
