// Copyright (c) 2013 CoreOS Authors. All rights reserved.
// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

#include "cgpt.h"
#include "vboot_host.h"

static void Usage(void)
{
  printf("\nUsage: %s next\n\n"
         "Look at all of the system disks and find the disk UUID we should attempt.\n"
         "\n"
         "The basic algorithm is to find the root partition with the highest priority\n"
         "and either a non-zero tries field or a non-zero successful field. Successful\n"
         "means it has been booted before and tries means it is a new update. Tries,\n"
         "if it exists, will be decremented after executing this command.\n"
         "\n"
         "The intended use of this command is in the initrd 'bootengine'."
         "\n", progname);
}

int cmd_next(int argc, char *argv[]) {
  CgptNextParams params;
  memset(&params, 0, sizeof(params));

  int c;
  int errorcnt = 0;

  opterr = 0;                     // quiet, you
  while ((c=getopt(argc, argv, ":hi:b:p")) != -1)
  {
    switch (c)
    {
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

  // TODO: handle drive types to make this generic

  return CgptNext(&params);
}
