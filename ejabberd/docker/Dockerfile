# This constructs a Docker image containing ejabberd (based mostly on
# the official ejabberd/ecs image) as well as the xidauth.py extauth
# script and the required Python runtime for it.

FROM ejabberd/ecs:20.04
USER root

RUN apk add --no-cache \
  python3 \
  py3-jsonrpclib

WORKDIR $HOME
COPY ejabberd/xidauth.py ejabberd/docker/xidauth.sh bin/

USER ejabberd
LABEL description="ejabberd/ecs image with XID authentication script"
