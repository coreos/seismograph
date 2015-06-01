/* Copyright (c) 2015 The CoreOS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Finds the size of an ext{2,3,4} filesystem located at the beginning of a given device.
 */

#include <stdio.h>
#include <inttypes.h>
#include <error.h>
#include <ext2fs/ext2fs.h>

static void usage() {
	fprintf(stderr, "Usage: e2size <device>\n");
}

int main(int argc, char *argv[]) {
	ext2_filsys fs;
	errcode_t err;
	uint64_t fs_size = 0;
	int retc = 1;

	if (argc != 2) {
		usage();
		goto out;
	}

	err = ext2fs_open(argv[1], 0, 0, 0, unix_io_manager, &fs);

	if (err != 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		goto out;
	}

	fs_size = ext2fs_blocks_count(fs->super) * fs->blocksize;

	printf("%" PRIu64 "\n", fs_size);

	retc = 0;

out:
	if ( fs != NULL ) {
		ext2fs_close(fs);
	}
	return retc;
}
