# Builds a Docker image that contains xid, and that will run the xid
# GSP process.  It will expose the HTTP REST interface as well as the
# ordinary JSON-RPC interface.

FROM xaya/libxayagame AS build
RUN apt-get install -y \
  autoconf \
  autoconf-archive \
  automake \
  build-essential \
  libtool \
  libcurl4-openssl-dev \
  libmicrohttpd-dev \
  pkg-config \
  protobuf-compiler

# Build and install xid.
WORKDIR /usr/src/xid
COPY . .
RUN make distclean || true
RUN ./autogen.sh && ./configure --disable-shared && make && make install-strip

# Copy the stuff we built to the final image.  We mainly need the xid
# GSP binary, but also provide the xidauth.py script so it can be extracted
# (or used) as needed to build an ejabberd image.
FROM xaya/libxayagame
COPY --from=build /usr/local/bin/xid /usr/local/bin/
COPY --from=build /usr/src/xid/ejabberd/xidauth.py /usr/local/share/
RUN apt-get install -y \
  libcurl4 \
  libmicrohttpd12 \
  && ldconfig
LABEL description="XAYA ID game-state processor"

# Define the runtime environment for XID.
RUN useradd xid \
  && mkdir -p /var/lib/xayagame && mkdir -p /var/log/xid \
  && chown xid:xid -R /var/lib/xayagame /var/log/xid
USER xid
VOLUME ["/var/lib/xayagame", "/var/log/xid"]
ENV GLOG_log_dir /var/log/xid
ENTRYPOINT [ \
  "/usr/local/bin/xid", \
  "--datadir=/var/lib/xayagame", \
  "--enable_pruning=1000" \
]
CMD []
