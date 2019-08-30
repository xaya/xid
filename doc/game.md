# The XID Game State and Move Format

This document gives a technical description of the underlying XAYA "game"
for XID.  In particular, the [game state](#game-state)
and [format for moves](#moves).

## <a id="game-state">Game State</a>

Internally, the state of XID is tracked in an
**[SQLite](https://sqlite.org/) database** as provided by the
[`SQLiteGame`](https://github.com/xaya/libxayagame/blob/master/xayagame/sqlitegame.hpp)
framework.  The exact format can be found in
[`schema.sql`](https://github.com/xaya/xid/blob/master/src/schema.sql).

### Signer Keys

Associations of *signer keys* are tracked by a table that contains triplets
of XAYA names, applications and signer addresses.  For each name,
there may be multiple rows in the table, so that multiple addresses can be
associated with it (as well as with multiple applications).  The application
column may be `NULL`.  Addresses in such rows are *global signers* instead of
being bound to particular applications.

Applications are identified by a string.  The developer of a particular
XID-enabled application has to make sure that the string is unique.
For instance, it can be based on a XAYA game ID with registered `g/` name,
or it can be based on a website's domain name.

**NOTE:**  The empty string `""` is a valid application name, although it should
not be used by a real-world application (as it is not descriptive / specific).
This is *not the same* as a `NULL` value, which represents global signers
instead!

### Crypto Addresses

Associations of crypto addresses to names are tracked simply by a table
that has rows of (name, crypto key, crypto address) triplets.  In that table,
(name, crypto key) is unique, i.e. there can be at most one address for
each "type" of crypto and name.

## <a id="moves">Move Format</a>

[Moves](https://github.com/xaya/xaya_docs/blob/master/games.md#moves)
in XID allow owners of XAYA names to change the data associated with their
names in the XID state.

The move data itself must be a **JSON object** of the following form:

    {
      "s":
        {
          "g": [GLOBAL1, GLOBAL2, ...],
          "a":
            {
              APP1: [ADDR1, ADDR2, ...],
              ...
            }
        },
      "ca":
        {
          CRYPTO1: CRYPTOADDR1,
          ...
        }
    }

Each of the top-level fields is optional.  When given, they specify
corresponding updates for the categories of data stored in XID.

### Signer Keys

If `s.g` is present, then the list of **global signers** for the name making
the move is set precisely to the given array of strings.  Similarly, for each
key `APP`i present in `s.a`, the list of signer keys **for application `APP`i**
is set precisely to the given list.

**NOTE:**  If `s.g` is not present or if some application name is not
a key in `s.a`, then the list of global signers or signers for that application
*is unchanged* (as opposed to set to an empty list).

Other keys in the top-level object or in `s` are ignored, as are values
that do not have the correct format (object, array, string).  All correctly
formatted parts of the move are still executed in that case.

### Crypto Addresses

For each `ca.CRYPTO`n field present, the address associated with the
move's name for the given type of crypto is updated.

If the `CRYPTOADDR`n is `null`, then the association is removed (i.e. there is
no longer an address stored for that crypto).  If `CRYPTOADDR`n is a string,
then an association to that string is created or an existing association
updated.  Otherwise (the value is neither `null` nor a string), nothing is done.

**NOTE:**  XID does not interpret either the crypto key or address in any
way.  As long as both are strings, XID performs the update without e.g.
validating that the address is valid for the given type of crypto.
