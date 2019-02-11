bin_PROGRAMS = xid

xid_CXXFLAGS = \
  $(XAYAGAME_CFLAGS) \
  $(JSON_CFLAGS) $(GLOG_CFLAGS) $(GFLAGS_CFLAGS)
xid_LDADD = \
  $(XAYAGAME_LIBS) \
  $(JSON_LIBS) \
  $(GLOG_LIBS) $(GFLAGS_LIBS)
xid_SOURCES = main.cpp \
  logic.cpp
noinst_HEADERS = \
  logic.hpp
