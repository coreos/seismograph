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

#include "cgpt.h"
#include "cgptlib_internal.h"
#include "vboot_host.h"

/* For building with linux headers < 3.6 */
#ifndef BLKPG_RESIZE_PARTITION
# define BLKPG_RESIZE_PARTITION 3
#endif


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

/* Find the whole disk device for a partition. */
static dev_t dev_to_wholedisk(blkid_dev dev) {
  dev_t devno, whole;

  if ((devno = dev_to_devno(dev)) < 0)
    return -1;

  if (blkid_devno_to_wholedisk(devno, NULL, 0, &whole) < 0)
    return -1;

  return whole;
}

/* Find the partition number for a given partition. */
static int dev_to_partno(blkid_dev dev) {
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
 *   CGPT_OK for resize successful
 *   CGPT_FAILED on error
 *   CGPT_NOOP if nothing to do
 */
static int resize_partition(CgptResizeParams *params, blkid_dev dev) {
  dev_t disk_devno;
  char disk_devname[512];
  struct drive drive;
  GptHeader *header;
  GptEntry *entry;
  int gpt_retval, entry_index, entry_count;
  uint64_t free_bytes, last_free_lba, entry_size_lba;

  if ((disk_devno = dev_to_wholedisk(dev)) < 0) {
    Error("Failed to find whole disk device for %s\n", blkid_dev_devname(dev));
    return CGPT_FAILED;
  }

  if (snprintf(disk_devname, sizeof(disk_devname), "/dev/block/%d:%d",
               major(disk_devno), minor(disk_devno)) <= 0)
    return CGPT_FAILED;

  if (DriveOpen(disk_devname, &drive, 0, O_RDWR) != CGPT_OK)
    return CGPT_FAILED;

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
  GptRepair(&drive.gpt);

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
      return CGPT_NOOP;
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

  UpdatePMBR(&drive);
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

/* a wrapper for the usual fork/exec boiler plate
 * returns process exit code or -1 on error */
static int xspawnlp(char *cmd, ...) {
  pid_t child_pid;
  int child_status, argno;
  char *args[10];
  va_list ap;

  va_start(ap, cmd);
  argno = 0;
  args[0] = cmd;
  while (args[argno] != NULL && argno < 9)
    args[++argno] = va_arg(ap, char *);
  args[argno] = NULL;
  va_end(ap);

  child_pid = fork();
  if (child_pid < 0) {
    Error("fork failed: %s\n", cmd, strerror(errno));
    return -1;
  }
  else if (child_pid == 0) {
    execvp(cmd, args);
    Error("exec %s failed: %s\n", cmd, strerror(errno));
    _exit(127);
  }
  else {
    if (waitpid(child_pid, &child_status, 0) < 0) {
      Error("waitpid for %s failed: %s\n", cmd, strerror(errno));
      return -1;
    }
    else if (WIFEXITED(child_status)) {
      return WEXITSTATUS(child_status);
    }
    else if (WIFSIGNALED(child_status)) {
      Error("process %s terminated by signal %s (%d)\n", cmd,
            strsignal(WTERMSIG(child_status)), WTERMSIG(child_status));
      return -1;
    }
  }

  return -1;
}

/* fsck and resize an ext[234] filesystem. */
static int resize_filesystem(CgptResizeParams *params, blkid_dev dev) {
  const char *devname = blkid_dev_devname(dev);
  int exit_code, err = CGPT_OK;

  // resize2fs won't run unless filesystem has been checked.
  exit_code = xspawnlp("e2fsck", "-f", "-p", devname, NULL);
  if (exit_code < 0)
    return CGPT_FAILED;
  else if (exit_code != 0 && exit_code != 1) {
    // only fs unchanged or fs repaired codes are acceptable.
    Error("e2fsck exited with code %d\n", exit_code);
    return CGPT_FAILED;
  }

  exit_code = xspawnlp("resize2fs", devname, NULL);
  if (exit_code < 0)
    err = CGPT_FAILED;
  else if (exit_code != 0) {
    Error("resize2fs exited with code %d\n", exit_code);
    err = CGPT_FAILED;
  }

  // make double sure resize2fs didn't make a mess of anything
  exit_code = xspawnlp("e2fsck", "-f", "-p", devname, NULL);
  if (exit_code < 0)
    return CGPT_FAILED;
  else if (exit_code != 0 && exit_code != 1) {
    // only fs unchanged or fs repaired codes are acceptable.
    Error("e2fsck exited with code %d\n", exit_code);
    return CGPT_FAILED;
  }

  return err;
}

/* Search for a partition to resize and expand it if possible.
 * Both the partition table and the filesytem will be updated.
 */
int CgptResize(CgptResizeParams *params) {
  blkid_cache cache = NULL;
  blkid_dev found_dev = NULL;
  char *tag_name = NULL, *tag_value = NULL;
  int err = CGPT_FAILED;

  if (params == NULL || params->partition_desc == NULL)
    return CGPT_FAILED;

  if (blkid_get_cache(&cache, NULL) < 0)
    goto exit;

  if (!strncmp(params->partition_desc, "/dev/", 5)) {
    found_dev = blkid_get_dev(cache, params->partition_desc, BLKID_DEV_NORMAL);
  }
  else {
    // No exact device specified, search all system partitions.
    Guid tag_guid;
    blkid_dev_iterate dev_iter;
    blkid_dev dev;

    if (blkid_parse_tag_string(params->partition_desc, &tag_name, &tag_value)) {
      Error("partition must be specified by a NAME=value pair\n");
      goto exit;
    }

    // Translate known partition types into plain GUID strings.
    if (strcmp(tag_name, "PARTTYPE") == 0 &&
        SupportedType(tag_value, &tag_guid) == CGPT_OK) {
      free(tag_value);
      tag_value = malloc(GUID_STRLEN);
      require(tag_value);
      GuidToStrLower(&tag_guid, tag_value, GUID_STRLEN);
    }

    blkid_probe_all(cache);
    dev_iter = blkid_dev_iterate_begin(cache);
    blkid_dev_set_search(dev_iter, tag_name, tag_value);

    // Check all devices, one and only one is allowed to match.
    while (blkid_dev_next(dev_iter, &dev) == 0) {
      dev = blkid_verify(cache, dev);
      if (!dev)
        continue;

      if (found_dev) {
        Error("more than one partition matched %s\n", params->partition_desc);
        goto exit;
      }

      found_dev = dev;
    }
  }

  if (!found_dev) {
    Error("no partition found matching %s\n", params->partition_desc);
    goto exit;
  }

  if ((err = resize_partition(params, found_dev)) != CGPT_OK)
    goto exit;

  // Only ext[123] filesystem resizing is supported.
  if (blkid_dev_has_tag(found_dev, "TYPE", "ext2") ||
      blkid_dev_has_tag(found_dev, "TYPE", "ext3") ||
      blkid_dev_has_tag(found_dev, "TYPE", "ext4")) {
    err = resize_filesystem(params, found_dev);
  }

exit:
  if (err == CGPT_NOOP)
    err = CGPT_OK;
  free(tag_name);
  free(tag_value);
  blkid_put_cache(cache);
  return err;
}
