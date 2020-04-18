# Light Mode

XID can be run in a special *light mode*, where it only supports
the minimum subset of the [RPC interface](rpc.md) required to generate
auth credentials.  This is useful in particular as the first step to
then using [Charon](https://github.com/xaya/charon) for gaming with
a fully light client.

The light mode implements
[`getauthmessage`](rpc.md#getauthmessage) and
[`setauthsignature`](rpc.md#setauthsignature) locally, as well as
[`getnullstate`](rpc.md#getnullstate) and
[`getnamestate`](rpc.md#getnamestate) by calling another (full) XID instance
using its [REST API](rest.md).

To start XID in light mode, the binary `xid-light` should be used
instead of `xid`.  It needs to be passed the local port for the reduced
JSON-RPC interface, and the endpoint of the remote REST API to use for
game-state requests.  For example:

    xid-light \
      --game_rpc_port=8400 \
      --rest_endpoint="https://seeder.xaya.io"
