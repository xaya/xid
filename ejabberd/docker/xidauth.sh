#!/bin/sh -e

# Copyright (C) 2020 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# This is a simple wrapper script around xidauth.py, using some environment
# variables from the Docker context to set the arguments.

exec ${HOME}/bin/xidauth.py \
  --xid_rpc_url "${XID_RPC_URL}" \
  --application "${XID_APPLICATION}" \
  --servername "${XMPP_DOMAIN}" \
  --logfile "${HOME}/logs/xidauth.log"
