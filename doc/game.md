# The Xid Game State and Move Format

This document gives a technical description of the underlying XAYA "game"
for Xid.  In particular, the [game state](#game-state)
and [format for moves](#moves).

## <a id="game-state">Game State</a>

Internally, the state of Xid is tracked in an
**[SQLite](https://sqlite.org/) database** as provided by the
[`SQLiteGame`](https://github.com/xaya/libxayagame/blob/master/xayagame/sqlitegame.hpp)
framework.  The exact format can be found in
[`schema.sql`](https://github.com/xaya/xid/blob/master/src/schema.sql).

Associations of *signer keys* are tracked by a table that contains triplets
of XAYA names, applications and signer addresses.  For each name,
there may be multiple rows in the table, so that multiple addresses can be
associated with it (as well as with multiple applications).  The application
column may be `NULL`.  Addresses in such rows are *global signers* instead of
being bound to particular applications.

Applications are identified by a string.  The developer of a particular
Xid-enabled application has to make sure that the string is unique.
For instance, it can be based on a XAYA game ID with registered `g/` name,
or it can be based on a website's domain name.

**NOTE:**  The empty string `""` is a valid application name, although it should
not be used by a real-world application (as it is not descriptive / specific).
This is *not the same* as a `NULL` value, which represents global signers
instead!

## <a id="moves">Move Format</a>

*[Moves](https://github.com/xaya/xaya_docs/blob/master/games.md#moves)
are not yet implemented and the format is not yet fully specified.*
