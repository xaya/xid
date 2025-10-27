#!/bin/sh -e

# Copyright (C) 2020-2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# This is a simple wrapper script around ejabbed_xidauth.py, using some
# environment variables from the Docker context to set the arguments.

. ${HOME}/venv/bin/activate
exec ${HOME}/bin/ejabberd_xidauth.py \
  --logfile "${HOME}/logs/xidauth.log" \
  $@
