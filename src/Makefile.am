noinst_LTLIBRARIES = libxid.la
bin_PROGRAMS = xid

EXTRA_DIST = schema.sql schema_head.cpp schema_tail.cpp
CLEANFILES = schema.cpp

libxid_la_CXXFLAGS = \
  $(JSON_CFLAGS) $(GLOG_CFLAGS) $(SQLITE3_CFLAGS)
libxid_la_LIBADD = \
  $(JSON_LIBS) $(GLOG_LIBS) $(SQLITE3_LIBS)
libxid_la_SOURCES = \
  database.cpp \
  gamestatejson.cpp \
  schema.cpp
libxidheaders = \
  database.hpp \
  gamestatejson.hpp \
  schema.hpp

xid_CXXFLAGS = \
  $(XAYAGAME_CFLAGS) \
  $(JSON_CFLAGS) $(GLOG_CFLAGS) $(GFLAGS_CFLAGS)
xid_LDADD = \
  $(builddir)/libxid.la \
  $(XAYAGAME_LIBS) \
  $(JSON_LIBS) $(GLOG_LIBS) $(GFLAGS_LIBS)
xid_SOURCES = main.cpp \
  logic.cpp
xidheaders = \
  logic.hpp

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
  schema_tests.cpp \
  \
  dbtest.cpp
check_HEADERS = \
  dbtest.hpp

schema.cpp: schema_head.cpp schema.sql schema_tail.cpp
	cat $^ >$@
