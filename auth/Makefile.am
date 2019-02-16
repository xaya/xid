lib_LTLIBRARIES = libxidauth.la
xidauthdir = $(includedir)/xidauth

EXTRA_DIST = auth.proto

BUILT_SOURCES = auth.pb.h
CLEANFILES = auth.pb.h auth.pb.cc

libxidauth_la_CXXFLAGS = \
  $(OPENSSL_CFLAGS) $(PROTOBUF_CFLAGS) $(GLOG_CFLAGS)
libxidauth_la_LIBADD = \
  $(OPENSSL_LIBS) $(PROTOBUF_LIBS) $(GLOG_LIBS)
libxidauth_la_SOURCES = \
  \
  auth.pb.cc
xidauth_HEADERS = \
  \
  auth.pb.h

auth.pb.h auth.pb.cc: $(srcdir)/auth.proto
	protoc --cpp_out=. "$<"
