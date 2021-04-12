noinst_LTLIBRARIES = libxid.la
bin_PROGRAMS = xid xid-light

EXTRA_DIST = \
  rpc-stubs/light.json \
  rpc-stubs/xid.json \
  schema.sql schema_head.cpp schema_tail.cpp

RPC_STUBS = \
  rpc-stubs/lightserverstub.h \
  rpc-stubs/xidrpcserverstub.h
BUILT_SOURCES = $(RPC_STUBS)
CLEANFILES = $(RPC_STUBS) schema.cpp

libxid_la_CXXFLAGS = \
  -I$(top_srcdir) \
  $(XAYAGAME_CFLAGS) \
  $(JSON_CFLAGS) $(GLOG_CFLAGS) $(SQLITE3_CFLAGS)
libxid_la_LIBADD = \
  $(top_builddir)/auth/libxidauth.la \
  $(XAYAGAME_LIBS) \
  $(JSON_LIBS) $(GLOG_LIBS) $(SQLITE3_LIBS)
libxid_la_SOURCES = \
  gamestatejson.cpp \
  light.cpp \
  moveprocessor.cpp \
  nonstaterpc.cpp \
  rpcerrors.cpp \
  schema.cpp
libxidheaders = \
  gamestatejson.hpp \
  light.hpp \
  moveprocessor.hpp \
  nonstaterpc.hpp \
  rpcerrors.hpp \
  schema.hpp

xid_CXXFLAGS = \
  -I$(top_srcdir) \
  $(XAYAUTIL_CFLAGS) $(XAYAGAME_CFLAGS) \
  $(JSON_CFLAGS) $(PROTOBUF_CFLAGS) $(GLOG_CFLAGS) $(GFLAGS_CFLAGS)
xid_LDADD = \
  $(builddir)/libxid.la \
  $(top_builddir)/auth/libxidauth.la \
  $(XAYAUTIL_LIBS) $(XAYAGAME_LIBS) \
  $(JSON_LIBS) $(PROTOBUF_LIBS) $(GLOG_LIBS) $(GFLAGS_LIBS)
xid_SOURCES = main-xid.cpp \
  logic.cpp \
  rest.cpp \
  xidrpcserver.cpp
xidheaders = \
  logic.hpp \
  rest.hpp \
  xidrpcserver.hpp \
  \
  rpc-stubs/xidrpcserverstub.h

xid_light_CXXFLAGS = \
  -I$(top_srcdir) \
  $(XAYAGAME_CFLAGS) \
  $(JSON_CFLAGS) \
  $(PROTOBUF_CFLAGS) $(GLOG_CFLAGS) $(GFLAGS_CFLAGS)
xid_light_LDADD = \
  $(builddir)/libxid.la \
  $(top_builddir)/auth/libxidauth.la \
  $(XAYAGAME_LIBS) \
  $(JSON_LIBS) \
  $(PROTOBUF_LIBS) $(GLOG_LIBS) $(GFLAGS_LIBS)
xid_light_SOURCES = main-light.cpp
lightheaders = rpc-stubs/lightserverstub.h

noinst_HEADERS = $(libxidheaders) $(xidheaders) $(lightheaders)

check_PROGRAMS = tests
TESTS = tests

tests_CXXFLAGS = \
  $(GTEST_MAIN_CFLAGS) \
  $(XAYAGAME_CFLAGS) \
  $(JSON_CFLAGS) $(GTEST_CFLAGS) $(GLOG_CFLAGS) $(SQLITE3_CFLAGS)
tests_LDADD = \
  $(builddir)/libxid.la \
  $(GTEST_MAIN_LIBS) \
  $(XAYAGAME_LIBS) \
  $(JSON_LIBS) $(GTEST_LIBS) $(GLOG_LIBS) $(SQLITE3_LIBS)
tests_SOURCES = \
  gamestatejson_tests.cpp \
  moveprocessor_tests.cpp \
  schema_tests.cpp \
  \
  dbtest.cpp \
  testutils.cpp
check_HEADERS = \
  dbtest.hpp \
  testutils.hpp

schema.cpp: schema_head.cpp schema.sql schema_tail.cpp
	cat $^ >$@

rpc-stubs/lightserverstub.h: $(srcdir)/rpc-stubs/light.json
	jsonrpcstub "$<" --cpp-server=LightServerStub --cpp-server-file="$@"
rpc-stubs/xidrpcserverstub.h: $(srcdir)/rpc-stubs/xid.json
	jsonrpcstub "$<" --cpp-server=XidRpcServerStub --cpp-server-file="$@"
