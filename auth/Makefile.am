lib_LTLIBRARIES = libxidauth.la
xidauthdir = $(includedir)/xidauth

EXTRA_DIST = auth.proto

BUILT_SOURCES = auth.pb.h
CLEANFILES = auth.pb.h auth.pb.cc

libxidauth_la_CXXFLAGS = \
  $(XAYAUTIL_CFLAGS) $(PROTOBUF_CFLAGS) $(GLOG_CFLAGS)
libxidauth_la_LIBADD = \
  $(XAYAUTIL_LIBS) $(PROTOBUF_LIBS) $(GLOG_LIBS)
libxidauth_la_SOURCES = \
  credentials.cpp \
  time.cpp \
  \
  auth.pb.cc
xidauth_HEADERS = \
  credentials.hpp \
  time.hpp \
  \
  auth.pb.h

check_PROGRAMS = tests
TESTS = tests

tests_CXXFLAGS = \
  $(GTEST_MAIN_CFLAGS) \
  $(XAYAUTIL_CFLAGS) $(PROTOBUF_CFLAGS) $(GLOG_CFLAGS)
tests_LDADD = \
  $(builddir)/libxidauth.la \
  $(GTEST_MAIN_LIBS) \
  $(XAYAUTIL_LIBS) $(PROTOBUF_LIBS) $(GLOG_LIBS)
tests_SOURCES = \
  credentials_tests.cpp \
  time_tests.cpp

auth.pb.h auth.pb.cc: $(srcdir)/auth.proto
	protoc --cpp_out=. "$<"
