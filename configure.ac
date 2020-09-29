AC_INIT([xid], [1.1])
AM_INIT_AUTOMAKE

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_SILENT_RULES([yes])

AC_PROG_CXX
AM_PROG_AR
AM_PATH_PYTHON([3.7])

LT_INIT

AC_LANG([C++])
AX_CXX_COMPILE_STDCXX([14], [noext])
AX_CHECK_COMPILE_FLAG([-Wall], [CXXFLAGS="${CXXFLAGS} -Wall"])
AX_CHECK_COMPILE_FLAG([-Werror], [CXXFLAGS="${CXXFLAGS} -Werror"])
AX_CHECK_COMPILE_FLAG([-pedantic], [CXXFLAGS="${CXXFLAGS} -pedantic"])
AX_CHECK_COMPILE_FLAG([-Wno-deprecated],
                      [CXXFLAGS="${CXXFLAGS} -Wno-deprecated"])

# Windows defines ERROR, which requires us to tell glog to not define
# it as abbreviated log severity (LOG(ERROR) still works, though, and
# that is all that we actually use in the code).
# See https://hpc.nih.gov/development/glog.html.
CXXFLAGS="${CXXFLAGS} -DGLOG_NO_ABBREVIATED_SEVERITIES"

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([XAYAGAME], [libxayagame])
PKG_CHECK_MODULES([XAYAUTIL], [libxayautil])
PKG_CHECK_MODULES([SQLITE3], [sqlite3])
PKG_CHECK_MODULES([JSON], [jsoncpp])
PKG_CHECK_MODULES([CURL], [libcurl])
PKG_CHECK_MODULES([OPENSSL], [openssl])
PKG_CHECK_MODULES([MHD], [libmicrohttpd])
PKG_CHECK_MODULES([GLOG], [libglog])
PKG_CHECK_MODULES([GTEST], [gtest])
PKG_CHECK_MODULES([GTEST_MAIN], [gtest_main])

# Make sure that pkg-config provides the right libraries for some dependencies
# that may only be available statically (e.g. gflags).  For context, see:
# https://bugs.freedesktop.org/show_bug.cgi?id=19541.
PKG_CONFIG="${PKG_CONFIG} --static"
PKG_CHECK_MODULES([GFLAGS], [gflags])
PKG_CHECK_MODULES([PROTOBUF], [protobuf])

AC_CONFIG_FILES([
  Makefile \
  auth/Makefile \
  data/Makefile \
  ejabberd/Makefile \
  gametest/Makefile \
  src/Makefile
])
AC_OUTPUT

echo
echo "CXXFLAGS: ${CXXFLAGS}"
