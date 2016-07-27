// Copyright (c) 2014 CoreOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <blkid/blkid.h>
#include <linux/blkpg.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "blkid_utils.h"
#include "cgpt.h"
#include "cgptlib_internal.h"
#include "vboot_host.h"

/* For building with linux headers < 3.6 */
#ifndef BLKPG_RESIZE_PARTITION
# define BLKPG_RESIZE_PARTITION 3
#endif

/* Call the BLKPG ioctl to notify the kernel of the change.
 * start and size are in bytes
 */
static int blkpg_resize_partition(int fd, int partno,
                                  uint64_t start, uint64_t size) {
  struct blkpg_ioctl_arg arg;
  struct blkpg_partition part;

  part.pno = partno;
  part.start = start;
  part.length = size;
  part.devname[0] = '\0';
  part.volname[0] = '\0';
  arg.op = BLKPG_RESIZE_PARTITION;
  arg.flags = 0;
  arg.datalen = sizeof(part);
  arg.data = &part;

  return ioctl(fd, BLKPG, &arg);
}

/* Resize the partition and notify the kernel.
 * returns:
 *   CGPT_OK for resize successful or nothing to do
 *   CGPT_FAILED on error
 */
static int resize_partition(CgptResizeParams *params, blkid_dev dev) {
  char *disk_devname;
  struct drive drive;
  GptHeader *header;
  GptEntry *entry;
  int gpt_retval, entry_index, entry_count;
  uint64_t free_bytes, last_free_lba, entry_size_lba;

  if ((disk_devname = dev_to_wholedevname(dev)) == NULL) {
    Error("Failed to find whole disk device for %s\n", blkid_dev_devname(dev));
    return CGPT_FAILED;
  }

  if (DriveOpen(disk_devname, &drive, 0, O_RDWR) != CGPT_OK) {
    free(disk_devname);
    return CGPT_FAILED;
  }

  free(disk_devname);

  if (CGPT_OK != ReadPMBR(&drive)) {
    Error("Unable to read PMBR\n");
    goto nope;
  }

  if (GPT_SUCCESS != (gpt_retval = GptSanityCheck(&drive.gpt))) {
    Error("GptSanityCheck() returned %d: %s\n",
          gpt_retval, GptError(gpt_retval));
    goto nope;
  }

  // If either table is bad fix it! (likely if disk was extended)
  if (GPT_SUCCESS != (gpt_retval = GptRepair(&drive.gpt))) {
    Error("GptRepair() returned %d: %s\n",
          gpt_retval, GptError(gpt_retval));
    goto nope;
  }

  header = (GptHeader*)drive.gpt.primary_header;
  last_free_lba = header->last_usable_lba;
  entry_count = GetNumberOfEntries(&drive);
  entry_index = dev_to_partno(dev) - 1;
  if (entry_index < 0 || entry_index >= entry_count) {
    Error("Kernel and GPT disagree on the number of partitions!\n");
    goto nope;
  }
  entry = GetEntry(&drive.gpt, PRIMARY, entry_index);

  // Scan entire table to determine if entry can grow.
  for (int i = 0; i < entry_count; i++) {
    GptEntry *other = GetEntry(&drive.gpt, PRIMARY, i);

    if (GuidIsZero(&other->type))
      continue;

    if (other->starting_lba > entry->ending_lba &&
        other->starting_lba - 1 < last_free_lba) {
      last_free_lba = other->starting_lba - 1;
    }
  }

  // Exit without doing anything if the size is too small
  free_bytes = (last_free_lba - entry->ending_lba) * drive.gpt.sector_bytes;
  if (entry->ending_lba >= last_free_lba ||
      free_bytes < params->min_resize_bytes) {
    if (DriveClose(&drive, 0) != CGPT_OK)
      return CGPT_FAILED;
    else
      return CGPT_OK;
  }

  // Update and test partition table in memory
  entry->ending_lba = last_free_lba;
  entry_size_lba = entry->ending_lba - entry->starting_lba;
  UpdateAllEntries(&drive);
  gpt_retval = CheckEntries((GptEntry*)drive.gpt.primary_entries,
                            (GptHeader*)drive.gpt.primary_header);
  if (gpt_retval != GPT_SUCCESS) {
    Error("CheckEntries() returned %d: %s\n",
          gpt_retval, GptError(gpt_retval));
    goto nope;
  }

  // Notify kernel of new partition size via an ioctl.
  if (blkpg_resize_partition(drive.fd, dev_to_partno(dev),
                             entry->starting_lba * drive.gpt.sector_bytes,
                             entry_size_lba * drive.gpt.sector_bytes) < 0) {
    Error("Failed to notify kernel of new partition size: %s\n"
          "Leaving existing partition table in place.\n", strerror(errno));
    goto nope;
  }

  UpdatePMBR(&drive, PRIMARY);
  if (WritePMBR(&drive) != CGPT_OK) {
    Error("Failed to write legacy MBR.\n");
    goto nope;
  }

  // Whew! we made it! Flush to disk.
  return DriveClose(&drive, 1);

nope:
  DriveClose(&drive, 0);
  return CGPT_FAILED;
}

/* Search for a partition to resize and expand it if possible.
 * Both the partition table and the filesytem will be updated.
 */
int CgptResize(CgptResizeParams *params) {
  blkid_cache cache = NULL;
  blkid_dev found_dev = NULL;
  int err = CGPT_FAILED;

  if (params == NULL || params->partition_desc == NULL)
    return CGPT_FAILED;

  if (blkid_get_cache(&cache, NULL) < 0)
    goto exit;

  found_dev = blkid_get_dev(cache, params->partition_desc, BLKID_DEV_NORMAL);

  if (!found_dev) {
    Error("device not found %s\n", params->partition_desc);
    goto exit;
  }

  err = resize_partition(params, found_dev);

exit:
  blkid_put_cache(cache);
  return err;
}
