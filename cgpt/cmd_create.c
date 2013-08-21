// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <getopt.h>
#include <string.h>

#include "cgpt.h"
#include "vboot_host.h"

static void Usage(void)
{
  printf("\nUsage: %s create [OPTIONS] DRIVE\n\n"
         "Create or reset an empty GPT.\n\n"
         "Options:\n"
         "  -c           Create disk image file if needed. Requires -s\n"
         "  -s NUM       Minimum disk sectors, extends image files\n"
         "  -z           Zero the sectors of the GPT table and entries\n"
         "\n", progname);
}

int cmd_create(int argc, char *argv[]) {
  CgptCreateParams params;
  memset(&params, 0, sizeof(params));

  int c;
  int errorcnt = 0;
  char *e = 0;

  opterr = 0;                     // quiet, you
  while ((c=getopt(argc, argv, ":hcs:z")) != -1)
  {
    switch (c)
    {
    case 'z':
      params.zap = 1;
      break;
    case 'c':
      params.create = 1;
      break;
    case 's':
      params.min_size = strtoull(optarg, &e, 0);
      if (!*optarg || (e && *e)) {
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
  if (params.create && !params.min_size) {
    Error("minimum size (-s) is required with create (-c)\n");
    errorcnt++;
  }
  if (errorcnt)
  {
    Usage();
    return CGPT_FAILED;
  }

  if (optind >= argc) {
    Usage();
    return CGPT_FAILED;
  }

  params.drive_name = argv[optind];

  return CgptCreate(&params);
}
