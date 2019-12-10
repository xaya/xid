# XID's REST API

In addition to the more standard [RPC interface](rpc.md), the XID daemon
can also expose some data through a read-only
[REST API](https://restfulapi.net/).  This allows e.g. setting up a public
web server that allows querying the XID state.

To enable the REST API, the argument `--rest_port=PORT` should be passed to
`xid` with a non-zero `PORT` value filled in.  This then starts a simple
HTTP server on all interfaces and the specified port, which serves the REST
API.

**Note that the HTTP server exposed by `xid` itself does not support TLS.
Thus to secure the API on a public server, one should probably put a reverse
proxy in front of `xid`'s REST port.**

## Server State

XID exposes its internal state (e.g. what block it is synced to) through
the API endpoing `/state`.  The result is simply a JSON value corresponding
to the result of [`getcurrentstate`](rpc.md#getcurrentstate), excluding
the `gamestate` field.

## Name State

The state of individual names (as per [`getnamestate`](rpc.md#getnamestate)) can
be retrieved through a query to `/name/NAME`.
