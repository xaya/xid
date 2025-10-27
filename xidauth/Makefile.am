AM_TESTS_ENVIRONMENT = \
  PYTHONPATH=$(top_srcdir):$(top_srcdir)/gametest

EXTRA_DIST = \
  abi/XayaAccounts.json \
  abi/XayaDelegation.json

BUILT_SOURCES = auth_pb2.py
CLEANFILES = auth_pb2.py

TEST_FILES = \
  tests/credentials.py \
  tests/delegation.py

noinst_PYTHON = \
  credentials.py \
  delegation.py \
  \
  $(TEST_FILES)

TESTS = $(TEST_FILES)

auth_pb2.py: $(top_srcdir)/auth/auth.proto
	protoc --python_out=. --proto_path=$(top_srcdir)/auth "$<"
