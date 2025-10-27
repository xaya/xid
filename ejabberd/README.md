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
**`ejabberd_xidauth.py` is such a script, which allows running `ejabberd` with
user authentication through Xid.**

## Usage of `ejabberd_xidauth.py`

[`ejabberd_xidauth.py`](https://github.com/xaya/xid/blob/master/ejabberd/ejabberd_xidauth.py)
is a self-contained Python script that implements
[`ejabberd` external
authentication](https://www.ejabberd.im/files/doc/dev.html#htoc9)
in a single source file.

It can be configured to support one or multiple servers.  For each
server, the following data bits need to be configured:

- The application name that should be used for
  [authentication with Xid](../doc/auth.md).  For simple usecases, this can
  just be chosen the same as the server name.
- The external name of the XMPP server
  for which authentication should be performed.  For instance, `chat.xaya.io`.

In addition, one or more authentication methods must be defined.  The server
accepts all users for which any one of them succeeds with their credentials:

- Authentication based on signers defined in the XID GSP state.  For this,
  the URL at which [`xid`'s JSON-RPC server](../doc/rpc.md) is listening
  must be configured.
- Authentication based on the Xaya delegation contract on an EVM chain,
  where signing allowances are managed with delegation permissions (and
  the account owner and all approved addresses are automatically signers).
  For this, the EVM node JSON-RPC URL and the address of the delegation
  contract to use must be configured.

The whole configuration must be encoded into a JSON object like this:

    {
      "server1": {
        "app": app1,
        "xid-gsp": rpc1,
      },
      "server2": {
        "app": app2,
        "delegation-contract": {
          "evm-rpc": rpc2,
          "del-addr": contract2
        },
      },
      ...
    }

This JSON object must be encoded into a string and stored in the
environment variable `EJABBERD_XIDAUTH_CONFIG`.

In addition, the following arguments can be passed optionally:

- `--help`:  Displays usage instructions on the command-line rather than
  starting the script.
- `--logfile=FILE`:  Sets the filename where logs should be written.
  By default, `/var/log/ejabberd/xidauth.log` is used.
- `--debug`:  Create more verbose debug logs instead of info level.

When started successfully, `ejabberd_xidauth.py` will simply run in an
endless loop,
reading commands from `stdin` and writing output to `stdout` (as per the
requirements for extauth scripts).

## Configuration of `ejabberd`

To use `ejabberd_xidauth.py` with `ejabberd`, the script has to be
copied to some
accessible place on the server first.  For instance, to `/etc/ejabberd`.
Then, the following configuration options should be set in `ejabberd.yml`:

    auth_method: external
    extauth_program: "/etc/ejabberd/ejabberd_xidauth.py ...arguments..."
