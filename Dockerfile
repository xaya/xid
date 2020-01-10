# Builds a Docker image that contains xid, and that will run the xid
# GSP process.  It will expose the HTTP REST interface as well as the
# ordinary JSON-RPC interface.

FROM xaya/libxayagame AS build
RUN apt-get update && apt-get install -y \
  autoconf \
  autoconf-archive \
  automake \
  libtool \
  pkg-config \
  protobuf-compiler

# Build and install xid.
WORKDIR /usr/src/xid
COPY . .
RUN ./autogen.sh && ./configure && make && make install

# Copy the stuff we built to the final image.
FROM xaya/libxayagame
COPY --from=build /usr/local /usr/local/
RUN ldconfig
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
