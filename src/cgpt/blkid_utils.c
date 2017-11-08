// Copyright (c) 2014 CoreOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <blkid/blkid.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include "cgpt.h"
#include "vboot_host.h"

/* Find the device id for a given blkid_dev.
 * FIXME: libblkid already has this info but lacks a function to expose it.
 */
static int dev_to_devno(blkid_dev dev, dev_t *devno) {
  struct stat dev_stat;

  if (stat(blkid_dev_devname(dev), &dev_stat) < 0)
    return -1;

  if (!S_ISBLK(dev_stat.st_mode))
    return -1;

  *devno = dev_stat.st_rdev;
  return 0;
}

/* Find the whole disk device path for a partition.
 * The resulting string must be freed. */
char * dev_to_wholedevname(blkid_dev dev) {
  dev_t devno, whole;

  if (dev_to_devno(dev, &devno) < 0)
    return NULL;

  if (blkid_devno_to_wholedisk(devno, NULL, 0, &whole) < 0)
    return NULL;

  return blkid_devno_to_devname(whole);
}

static int devno_to_partno(dev_t devno) {
  char sys_path[512];
  FILE *sys_fd;
  int partno;

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

/* Find the partition number for a given partition. */
int dev_to_partno(blkid_dev dev) {
  dev_t devno;

  if (dev_to_devno(dev, &devno) < 0)
    return -1;

  return devno_to_partno(devno);
}

/* Utility for easily accepting whole disk or partition devices.
 * If drive path is set but partition is 0 the drive path will be checked
 * to see if it is a partition device instead of a whole disk. If it is a
 * partition device it is changed to the whole disk device and the partition
 * number is filled in. */
int translate_partition_dev(char **devname, uint32_t *partition) {
  struct stat dev_stat;
  dev_t whole_devno;
  char *whole_devname;
  int partno;

  require(devname && *devname && partition);

  if (!strlen(*devname)) {
    Error("empty drive argument\n");
    return CGPT_FAILED;
  }

  if (stat(*devname, &dev_stat) < 0) {
    Error("unable to access device %s: %s\n", *devname, strerror(errno));
    return CGPT_FAILED;
  }

  /* No magic to be done if this isn't a block device */
  if (!S_ISBLK(dev_stat.st_mode))
    return CGPT_OK;

  if (blkid_devno_to_wholedisk(dev_stat.st_rdev, NULL, 0, &whole_devno) < 0) {
    Error("unable to map %s to a whole disk device\n", *devname);
    return CGPT_FAILED;
  }

  /* Nothing to check or translate if we already have the whole disk */
  if (dev_stat.st_rdev == whole_devno)
    return CGPT_OK;

  if ((partno = devno_to_partno(dev_stat.st_rdev)) < 0) {
    Error("unable to look up partition number for %s\n", *devname);
    return CGPT_FAILED;
  }

  if (*partition && *partition != partno) {
    Error("device %s is partition %d but %d was specified\n",
          *devname, partno, *partition);
    return CGPT_FAILED;
  }

  if ((whole_devname = blkid_devno_to_devname(whole_devno)) == NULL) {
    Error("unable to map %s to a whole disk device name\n", *devname);
    return CGPT_FAILED;
  }

  free(*devname);
  *devname = whole_devname;
  *partition = partno;
  return CGPT_OK;
}
