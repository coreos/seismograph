// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <string.h>

#include "cgpt.h"
#include "cgptlib_internal.h"
#include "vboot_host.h"

int CgptRepair(CgptRepairParams *params) {
  struct drive drive;

  if (params == NULL)
    return CGPT_FAILED;

  if (CGPT_OK != DriveOpen(params->drive_name, &drive, 0, O_RDWR))
    return CGPT_FAILED;

  if (CGPT_OK != ReadPMBR(&drive)) {
    Error("Unable to read PMBR\n");
  }

  int gpt_retval = GptSanityCheck(&drive.gpt);
  if (params->verbose)
    printf("GptSanityCheck() returned %d: %s\n",
           gpt_retval, GptError(gpt_retval));

  if (GPT_SUCCESS != (gpt_retval = GptRepair(&drive.gpt))) {
    Error("GptRepair() returned %d: %s\n",
          gpt_retval, GptError(gpt_retval));
    return CGPT_FAILED;
  }

  if (drive.gpt.modified & GPT_MODIFIED_HEADER1)
    printf("Primary Header is updated.\n");
  if (drive.gpt.modified & GPT_MODIFIED_ENTRIES1)
    printf("Primary Entries is updated.\n");
  if (drive.gpt.modified & GPT_MODIFIED_ENTRIES2)
    printf("Secondary Entries is updated.\n");
  if (drive.gpt.modified & GPT_MODIFIED_HEADER2)
    printf("Secondary Header is updated.\n");

  UpdatePMBR(&drive, ANY_VALID);
  if (WritePMBR(&drive) != CGPT_OK) {
    Error("Failed to write legacy MBR.\n");
  }

  return DriveClose(&drive, 1);
}
