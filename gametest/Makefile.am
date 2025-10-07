AM_TESTS_ENVIRONMENT = \
  top_builddir=$(top_builddir) \
  top_srcdir=$(top_srcdir)

TEST_LIBRARY = \
  xidtest.py

REGTESTS = \
  address_update.py \
  auth.py \
  getnamestate.py \
  light.py \
  rest.py \
  signer_update.py

EXTRA_DIST = $(REGTESTS) $(TEST_LIBRARY)
TESTS = $(REGTESTS)
