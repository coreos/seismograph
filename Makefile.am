AM_MAKEFLAGS = --no-print-directory
# remove targets if the command fails
.DELETE_ON_ERROR:

EXTRA_DIST =

AM_CPPFLAGS = -I$(srcdir)/firmware/include \
	      -I$(srcdir)/firmware/lib/include \
	      -I$(srcdir)/firmware/lib/cgptlib/include \
	      -I$(srcdir)/host/include
AM_CFLAGS = -Wall -Werror \
	    -fvisibility=hidden \
	    -std=gnu99

bin_PROGRAMS = cgpt

cgpt_SOURCES = \
	cgpt/blkid_utils.c \
	cgpt/cgpt_add.c \
	cgpt/cgpt_boot.c \
	cgpt/cgpt.c \
	cgpt/cgpt_common.c \
	cgpt/cgpt_create.c \
	cgpt/cgpt_find.c \
	cgpt/cgpt_legacy.c \
	cgpt/cgpt_next.c \
	cgpt/cgpt_prioritize.c \
	cgpt/cgpt_repair.c \
	cgpt/cgpt_resize.c \
	cgpt/cgpt_show.c \
	cgpt/cmd_add.c \
	cgpt/cmd_boot.c \
	cgpt/cmd_create.c \
	cgpt/cmd_find.c \
	cgpt/cmd_legacy.c \
	cgpt/cmd_next.c \
	cgpt/cmd_prioritize.c \
	cgpt/cmd_repair.c \
	cgpt/cmd_resize.c \
	cgpt/cmd_show.c \
	firmware/lib/cgptlib/cgptlib.c \
	firmware/lib/cgptlib/cgptlib_internal.c \
	firmware/lib/cgptlib/crc32.c \
	firmware/lib/utility.c \
	firmware/lib/utility_string.c \
	firmware/stub/utility_stub.c
cgpt_LDADD = $(BLKID_LIBS) $(UUID_LIBS)

check_PROGRAMS = cgptlib_test \
		 utility_string_tests \
		 utility_tests
EXTRA_DIST += tests/common.sh \
	      tests/run_cgpt_tests.sh
TESTS = cgptlib_test \
	utility_string_tests \
	utility_tests \
	tests/run_cgpt_tests.sh
TESTS_ENVIRONMENT = export BUILD=$(builddir);

cgptlib_test_SOURCES = tests/cgptlib_test.c \
		       tests/crc32_test.c \
		       tests/test_common.c \
		       firmware/lib/cgptlib/cgptlib.c \
		       firmware/lib/cgptlib/cgptlib_internal.c \
		       firmware/lib/cgptlib/crc32.c \
		       firmware/lib/utility.c \
		       firmware/lib/utility_string.c \
		       firmware/stub/utility_stub.c
utility_string_tests_SOURCES = tests/utility_string_tests.c \
			       tests/test_common.c \
			       firmware/lib/utility_string.c \
			       firmware/stub/utility_stub.c
utility_tests_SOURCES = tests/utility_tests.c \
			tests/test_common.c \
			firmware/lib/utility.c
