# This constructs a Docker image containing ejabberd (based mostly on
# the official ejabberd/ecs image) as well as the ejabberd_xidauth.py extauth
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
  protobuf \
  python3 \
  py3-pip \
  py3-virtualenv

WORKDIR $HOME

COPY xidauth/ xid/xidauth/
COPY pyproject.toml requirements.txt auth/auth.proto xid/

# Make sure the Python protobuf file is generated from source in case it
# isn't in the context.
RUN rm -f xid/xidauth/auth_pb2.py \
    && protoc --python_out=xid/xidauth --proto_path=xid xid/auth.proto

RUN virtualenv venv && . venv/bin/activate \
    && pip3 install -r xid/requirements.txt \
    && pip3 install ./xid
COPY ejabberd/ejabberd_xidauth.py ejabberd/docker/ejabberd_xidauth.sh bin/

# Install custom ejabberd module via ext_mod directory structure.
# ejabberd compiles and loads it automatically on startup.
RUN mkdir -p .ejabberd-modules/sources/mod_archive_contacts/src
COPY ejabberd/mod_archive_contacts.erl \
     .ejabberd-modules/sources/mod_archive_contacts/src/

USER ejabberd
LABEL description="ejabberd/ecs image with XID authentication script"
