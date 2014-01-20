// Copyright (c) 2014 CoreOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgpt.h"
#include "vboot_host.h"

static void Usage(void)
{
  printf("\nUsage: %s rezise [TAG=partsearch | /dev/blk1]\n\n"
         "Search all system disks for a partition to resize. If a specific\n"
         "device node in /dev is given then use that instead of searching.\n"
         "To search any unique tag reported by `blkid` can be used.\n"
         "If more than one partition is found, the partition has not enough\n"
         "room to grow, or the partition's filesystem is not ext2+ then this\n"
         "command will return without making any changes to the system.\n\n"
         "The default search is PARTTYPE=coreos-resize.\n"
         "The default minimum size to grow by is 2MB.\n\n"
         "Options:\n"
         "  -m NUM       Do nothing unless partition can grow by NUM bytes\n"
         "\n", progname);
}

int cmd_resize(int argc, char *argv[]) {
  CgptResizeParams params;
  memset(&params, 0, sizeof(params));

  // Default minimum is 2MB
  params.min_resize_bytes = 2 * 1024 * 1024;

  int c;
  int errorcnt = 0;
  char *e = 0;

  opterr = 0;                     // quiet, you
  while ((c=getopt(argc, argv, ":d:m:h")) != -1)
  {
    switch (c)
    {
    case 'm':
      params.min_resize_bytes = (uint32_t)strtoull(optarg, &e, 0);
      if (!*optarg || (e && *e))
      {
        Error("invalid argument to -%c: \"%s\"\n", c, optarg);
        errorcnt++;
      }
      break;

    case 'h':
      Usage();
      return CGPT_OK;
    case '?':
      Error("unrecognized option: -%c\n", optopt);
      errorcnt++;
      break;
    case ':':
      Error("missing argument to -%c\n", optopt);
      errorcnt++;
      break;
    default:
      errorcnt++;
      break;
    }
  }
  if (errorcnt)
  {
    Usage();
    return CGPT_FAILED;
  }

  if (optind < argc) {
    params.partition_desc = argv[optind];
  } else {
    params.partition_desc = "PARTTYPE=coreos-resize";
  }

  return CgptResize(&params);
}
