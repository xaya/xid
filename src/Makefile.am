noinst_LTLIBRARIES = libxid.la
bin_PROGRAMS = xid

EXTRA_DIST = \
  rpc-stubs/xaya-wallet.json \
  rpc-stubs/xid.json \
  schema.sql schema_head.cpp schema_tail.cpp

RPC_STUBS = \
  rpc-stubs/xaya-wallet-rpcclient.h \
  rpc-stubs/xidrpcserverstub.h
BUILT_SOURCES = $(RPC_STUBS)
CLEANFILES = $(RPC_STUBS) schema.cpp

libxid_la_CXXFLAGS = \
  $(JSON_CFLAGS) $(GLOG_CFLAGS) $(SQLITE3_CFLAGS)
libxid_la_LIBADD = \
  $(JSON_LIBS) $(GLOG_LIBS) $(SQLITE3_LIBS)
libxid_la_SOURCES = \
  database.cpp \
  gamestatejson.cpp \
  moveprocessor.cpp \
  schema.cpp
libxidheaders = \
  database.hpp \
  gamestatejson.hpp \
  moveprocessor.hpp \
  schema.hpp

xid_CXXFLAGS = \
  -I$(top_srcdir) \
  $(XAYAGAME_CFLAGS) \
  $(JSON_CFLAGS) $(PROTOBUF_CFLAGS) $(GLOG_CFLAGS) $(GFLAGS_CFLAGS)
xid_LDADD = \
  $(builddir)/libxid.la \
  $(top_builddir)/auth/libxidauth.la \
  $(XAYAGAME_LIBS) \
  $(JSON_LIBS) $(PROTOBUF_LIBS) $(GLOG_LIBS) $(GFLAGS_LIBS)
xid_SOURCES = main.cpp \
  logic.cpp \
  xidrpcserver.cpp
xidheaders = \
  logic.hpp \
  xidrpcserver.hpp \
  \
  rpc-stubs/xaya-wallet-rpcclient.h \
  rpc-stubs/xidrpcserverstub.h

noinst_HEADERS = $(libxidheaders) $(xidheaders)

check_PROGRAMS = tests
TESTS = tests

tests_CXXFLAGS = \
  $(GTEST_MAIN_CFLAGS) \
  $(JSON_CFLAGS) $(GTEST_CFLAGS) $(GLOG_CFLAGS) $(SQLITE3_CFLAGS)
tests_LDADD = \
  $(builddir)/libxid.la \
  $(GTEST_MAIN_LIBS) \
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

rpc-stubs/xaya-wallet-rpcclient.h: $(srcdir)/rpc-stubs/xaya-wallet.json
	jsonrpcstub "$<" --cpp-client=XayaWalletRpcClient --cpp-client-file="$@"
rpc-stubs/xidrpcserverstub.h: $(srcdir)/rpc-stubs/xid.json
	jsonrpcstub "$<" --cpp-server=XidRpcServerStub --cpp-server-file="$@"
