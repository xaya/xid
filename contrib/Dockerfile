# Builds a Docker image that contains xid, and that will run the xid
# GSP process.  It will expose the HTTP REST interface as well as the
# ordinary JSON-RPC interface.

FROM xaya/libxayagame AS build
RUN apt-get install -y \
  autoconf \
  autoconf-archive \
  automake \
  build-essential \
  git \
  libtool \
  libcurl4-openssl-dev \
  libmicrohttpd-dev \
  pkg-config \
  protobuf-compiler

# Build and install xid.
WORKDIR /usr/src/xid
COPY . .
RUN make distclean || true
RUN ./autogen.sh && ./configure && make && make install-strip
RUN ldconfig

# Collect just the minimal required stuff to run xid (binary plus
# necessary shared libraries).
WORKDIR /jail
RUN mkdir bin && cp /usr/local/bin/xid bin/
RUN cpld bin/xid lib64
RUN mkdir xayagame log

# Build the final image.
FROM scratch
COPY --from=build /jail /
ENV LD_LIBRARY_PATH=/lib64/
LABEL description="XAYA ID game-state processor"
VOLUME ["/xayagame", "/log"]
ENV GLOG_log_dir /log
ENTRYPOINT [ \
  "/bin/xid", \
  "--datadir=/xayagame", \
  "--enable_pruning=1000" \
]
CMD []
