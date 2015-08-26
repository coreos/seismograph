/* Copyright (c) 2015 CoreOS. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <sys/mount.h>
#include <linux/loop.h>
#include <sys/file.h>
#include <libmount/libmount.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static int get_node(const char *image_path, char **loopdev, int *loop) {
	

        struct loop_info64 info = {
                .lo_flags = LO_FLAGS_AUTOCLEAR
        };

	int nr = -1, fd = -1, control = -1, success = -1;

        fd = open(image_path, O_CLOEXEC|O_RDWR);
        if (fd < 0) {
                perror("Failed to open file descriptor for provided image");
                goto out;
        }

        control = open("/dev/loop-control", O_RDWR|O_CLOEXEC);
        if (control < 0) {
                perror("Failed to open /dev/loop-control");
                goto out;
        }

        nr = ioctl(control, LOOP_CTL_GET_FREE);
        if (nr < 0) {
                perror("Failed to allocate loop device node");
                goto out;
        }

         if (asprintf(loopdev, "/dev/loop%i", nr) < 0) {
                perror("Failed to retrieve path of loop device node");
                goto out;
        }

        printf("initializing loop device node at /dev/loop%d \n", nr);

        *loop = open(*loopdev, O_CLOEXEC|O_RDWR);
        if (*loop < 0) {
                perror("Failed to open loop device loopdev");
                goto out;
        }

        if (ioctl(*loop, LOOP_SET_FD, fd) < 0) {
                perror("Failed to set loopback file descriptor fd on loopdev");
                goto out;
        }

	if (ioctl(*loop, LOOP_SET_STATUS64, &info) < 0) {
		perror("Failed to set loop device status on new node");
		goto out;
	}

        success = 1;

        out:

                if (fd >= 0)
                        close(fd);

                if (success != 1 && nr >= 0 && control >= 0)
                        ioctl(control, LOOP_CTL_REMOVE, nr);

                if (control >= 0)
                        close(control);
                
                return success;

}

static int single_mount(const char *image_path, const char *target) {


        int rc = -1;
	int loop = -1;
        struct libmnt_context *cxt;
        cxt = mnt_new_context();
        char *source = NULL;

        if ( get_node(image_path, &source, &loop) < 0 ) {
                printf("Failed to get node for mounting");
                goto out;
        }

        if ( mnt_context_set_source(cxt, source) < 0 ) {
                perror("Failed to set mount device source");
                goto out; 
        }
        
        if ( mnt_context_set_target(cxt, target) < 0 ) {
                perror("Failed to set mount target directory");
                goto out;
        }

        rc = mnt_context_mount(cxt);
        if (rc) {
                if (rc > 0) {
                        perror("Failure to execute mount. Mount(2) error in syscall");
                        goto out;
                }
                else {
                        perror("Failure to execute mount. Error not related to Mount(2)");
                        goto out; 
                }
        }
	
        
        out:
		if (rc > 0) 
			rc = -1;
		
		mnt_free_context(cxt);
        
	       	free(source);

		if (loop >= 0)
			close(loop);

        return rc;
}

int main(int argc, char *argv[]) {

        const char *path, *target;
        
	if (!(argc == 3)) {
                perror("Invalid usage of loop_util. Please input <SOURCE> <TARGET>");
                return -1;
        }

        path = argv[1];
        target = argv[2];

        int p = single_mount(path, target);
        if (p < 0)
                fprintf(stderr, "Failure to execute single_mount");
        return p;

}


