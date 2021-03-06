# Builds a Docker image that contains xid, and that will run the xid
# GSP process.  It will expose the HTTP REST interface as well as the
# ordinary JSON-RPC interface.

FROM xaya/libxayagame AS build
RUN apk add --no-cache \
  autoconf \
  autoconf-archive \
  automake \
  build-base \
  gflags-dev \
  libtool \
  pkgconfig

# Build and install xid.
WORKDIR /usr/src/xid
COPY . .
RUN make distclean || true
RUN ./autogen.sh && ./configure && make && make install-strip

# Collect just the minimal required stuff to run xid (binary plus
# necessary shared libraries).
WORKDIR /jail
RUN mkdir bin && cp /usr/local/bin/xid bin/
RUN cpld bin/xid lib64

# Build the final image.
FROM alpine
COPY --from=build /jail /usr/local/
ENV LD_LIBRARY_PATH "/usr/local/lib64"
LABEL description="XAYA ID game-state processor"
VOLUME ["/xayagame", "/log"]
ENV GLOG_log_dir /log
ENTRYPOINT [ \
  "/usr/local/bin/xid", \
  "--datadir=/xayagame", \
  "--enable_pruning=1000" \
]
