# This constructs a Docker image containing ejabberd (based mostly on
# the official ejabberd/ecs image) as well as the xidauth.py extauth
# script and the required Python runtime for it.

# To bypass https://github.com/moby/moby/issues/3465, we use docker-copyedit
# from https://github.com/gdraheim/docker-copyedit to create the image used
# here from ejabberd/ecs in a first step.
#
# With this, run the following command to construct ejabberd-unvolumed:
#   docker-copyedit.py FROM ejabberd/ecs INTO ejabbed-unvolumed \
#     REMOVE ALL VOLUMES

FROM ejabberd-unvolumed
USER root

RUN apk add --no-cache \
  python3 \
  py3-jsonrpclib

WORKDIR $HOME
COPY ejabberd/xidauth.py ejabberd/docker/xidauth.sh bin/

USER ejabberd
LABEL description="ejabberd/ecs image with XID authentication script"
