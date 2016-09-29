ACLOCAL_AMFLAGS = -I m4
AM_MAKEFLAGS = --no-print-directory
# remove targets if the command fails
.DELETE_ON_ERROR:

EXTRA_DIST =

AM_CPPFLAGS = -I$(srcdir)/src/firmware/include \
	      -I$(srcdir)/src/firmware/lib/include \
	      -I$(srcdir)/src/firmware/lib/cgptlib/include \
	      -I$(srcdir)/src/host/include \
	      -I$(srcdir)/include
AM_CFLAGS = -Wall -Werror \
	    -fvisibility=hidden \
	    -std=gnu99

bin_PROGRAMS = cgpt e2size rootdev
lib_LTLIBRARIES = librootdev.la

if ENABLE_LOOPY
bin_PROGRAMS += loopy
endif

rootdevincludedir = $(includedir)/rootdev
rootdevinclude_HEADERS = include/rootdev/rootdev.h

cgpt_SOURCES = \
	src/cgpt/blkid_utils.c \
	src/cgpt/cgpt_add.c \
	src/cgpt/cgpt_boot.c \
	src/cgpt/cgpt.c \
	src/cgpt/cgpt_common.c \
	src/cgpt/cgpt_create.c \
	src/cgpt/cgpt_find.c \
	src/cgpt/cgpt_legacy.c \
	src/cgpt/cgpt_next.c \
	src/cgpt/cgpt_prioritize.c \
	src/cgpt/cgpt_repair.c \
	src/cgpt/cgpt_resize.c \
	src/cgpt/cgpt_show.c \
	src/cgpt/cmd_add.c \
	src/cgpt/cmd_boot.c \
	src/cgpt/cmd_create.c \
	src/cgpt/cmd_find.c \
	src/cgpt/cmd_legacy.c \
	src/cgpt/cmd_next.c \
	src/cgpt/cmd_prioritize.c \
	src/cgpt/cmd_repair.c \
	src/cgpt/cmd_resize.c \
	src/cgpt/cmd_show.c \
	src/firmware/lib/cgptlib/cgptlib.c \
	src/firmware/lib/cgptlib/cgptlib_internal.c \
	src/firmware/lib/cgptlib/crc32.c \
	src/firmware/lib/utility.c \
	src/firmware/lib/utility_string.c \
	src/firmware/stub/utility_stub.c
cgpt_LDADD = $(BLKID_LIBS) $(UUID_LIBS)

e2size_SOURCES = src/e2size/e2size.c
e2size_LDADD = $(EXT2FS_LIBS)

loopy_SOURCES = src/loopy/loopy.c
loopy_LDADD = $(MNT_LIBS)

librootdev_la_SOURCES = src/rootdev/rootdev.c
librootdev_la_CFLAGS = -Wall -Werror -std=gnu99
librootdev_la_LDFLAGS = -export-symbols-regex '^rootdev' \
			-version-info 1:0:0

rootdev_SOURCES = src/rootdev/main.c
rootdev_LDADD = librootdev.la

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

cgptlib_test_SOURCES = \
	tests/cgptlib_test.c \
	tests/crc32_test.c \
	tests/test_common.c \
	src/firmware/lib/cgptlib/cgptlib.c \
	src/firmware/lib/cgptlib/cgptlib_internal.c \
	src/firmware/lib/cgptlib/crc32.c \
	src/firmware/lib/utility.c \
	src/firmware/lib/utility_string.c \
	src/firmware/stub/utility_stub.c
utility_string_tests_SOURCES = \
	tests/utility_string_tests.c \
	tests/test_common.c \
	src/firmware/lib/utility_string.c \
	src/firmware/stub/utility_stub.c
utility_tests_SOURCES = \
	tests/utility_tests.c \
	tests/test_common.c \
	src/firmware/lib/utility.c
