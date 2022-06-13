# xid Authentication for `ejabberd`

[XMPP](https://xmpp.org/) is a federated and open protocol for
chat and messaging systems.  It can be particularly useful in games,
both to support chat between players and for automated communcation.
The latter can be used, for instance, to negotiate and execute
[player-to-player
trades](https://github.com/xaya/xaya/blob/master/doc/xaya/trading.md)
or for other off-chain interactions (like
[game channels](http://ledgerjournal.org/ojs/index.php/ledger/article/view/15)).
[`ejabberd`](https://www.ejabberd.im/) is a free and widely used
implementation of an XMPP server.

For ideal integration of XMPP services with the XAYA ecosystem, servers
should use Xid identities for [user authentication](../doc/auth.md)
(so that no separate user account is necessary).
Luckily, `ejabberd` supports pluggable scripts for
[external authentication](https://www.ejabberd.im/files/doc/dev.html#htoc9).
**`xidauth.py` is such a script, which allows running `ejabberd` with
user authentication through Xid.**

## Usage of `xidauth.py`

[`xidauth.py`](https://github.com/xaya/xid/blob/master/ejabberd/xidauth.py)
is a self-contained Python script that implements
[`ejabberd` external
authentication](https://www.ejabberd.im/files/doc/dev.html#htoc9)
in a single source file.

It requires the following command-line arguments to be set:

- `--xid_rpc_url=URL`:  Must be set to the URL at which
  [`xid`'s JSON-RPC server](../doc/rpc.md) is listening.
  For instance, `http://localhost:PORT` for `xid` running locally
  on the given port.
- `--servername=SERVER`:  This specifies the external name of the XMPP server
  for which authentication should be performed.  For instance, `chat.xaya.io`.
- `--application=APP`:  The application name that should be used for
  [authentication with Xid](../doc/auth.md).  For simple usecases, this can
  just be chosen the same as `--servername`.

In addition, the following arguments can be passed optionally:

- `--help`:  Displays usage instructions on the command-line rather than
  starting the script.
- `--logfile=FILE`:  Sets the filename where logs should be written.
  By default, `/var/log/ejabberd/xidauth.log` is used.

When started successfully, `xidauth.py` will simply run in an endless loop,
reading commands from `stdin` and writing output to `stdout` (as per the
requirements for extauth scripts).

## Configuration of `ejabberd`

To use `xidauth.py` with `ejabberd`, the script has to be copied to some
accessible place on the server first.  For instance, to `/etc/ejabberd`.
Then, the following configuration options should be set in `ejabberd.yml`:

    auth_method: external
    extauth_program: "/etc/ejabberd/xidauth.py ...arguments..."
