AM_TESTS_ENVIRONMENT = \
  PYTHONPATH=$(top_srcdir)

BUILT_SOURCES = auth_pb2.py
CLEANFILES = auth_pb2.py

TEST_FILES = \
  tests/credentials.py

noinst_PYTHON = \
  credentials.py \
  \
  $(TEST_FILES)

TESTS = $(TEST_FILES)

auth_pb2.py: $(top_srcdir)/auth/auth.proto
	protoc --python_out=. --proto_path=$(top_srcdir)/auth "$<"
