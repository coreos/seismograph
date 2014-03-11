// Copyright (c) 2014 CoreOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <blkid/blkid.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Find the device id for a given blkid_dev.
 * FIXME: libblkid already has this info but lacks a function to expose it.
 */
static dev_t dev_to_devno(blkid_dev dev) {
  struct stat dev_stat;

  if (stat(blkid_dev_devname(dev), &dev_stat) < 0)
    return -1;

  if (!S_ISBLK(dev_stat.st_mode))
    return -1;

  return dev_stat.st_rdev;
}

/* Find the whole disk device path for a partition.
 * The resulting string must be freed. */
char * dev_to_wholedevname(blkid_dev dev) {
  dev_t devno, whole;

  if ((devno = dev_to_devno(dev)) < 0)
    return NULL;

  if (blkid_devno_to_wholedisk(devno, NULL, 0, &whole) < 0)
    return NULL;

  return blkid_devno_to_devname(whole);
}

/* Find the partition number for a given partition. */
int dev_to_partno(blkid_dev dev) {
  char sys_path[512];
  FILE *sys_fd;
  int devno, partno;

  if ((devno = dev_to_devno(dev)) < 0)
    return -1;

  if (snprintf(sys_path, sizeof(sys_path),
               "/sys/dev/block/%d:%d/partition",
               major(devno), minor(devno)) <= 0)
    return -1;

  if ((sys_fd = fopen(sys_path, "r")) == NULL)
    return -1;

  if (fscanf(sys_fd, "%d", &partno) != 1)
    partno = -1;

  fclose(sys_fd);
  return partno;
}
