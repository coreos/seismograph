// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "cgpt.h"
#include "cgpt_params.h"
#include "cgptlib_internal.h"
#include "endian.h"
#include "vboot_host.h"

int CgptGetBootPartitionNumber(CgptBootParams *params) {
  struct drive drive;
  int gpt_retval= 0;
  int retval;

  if (params == NULL)
    return CGPT_FAILED;

  if (CGPT_OK != DriveOpen(params->drive_name, &drive, 0, O_RDONLY))
    return CGPT_FAILED;

  if (GPT_SUCCESS != (gpt_retval = GptSanityCheck(&drive.gpt))) {
    Error("GptSanityCheck() returned %d: %s\n",
          gpt_retval, GptError(gpt_retval));
    retval = CGPT_FAILED;
    goto done;
  }

  if (CGPT_OK != ReadPMBR(&drive)) {
    Error("Unable to read PMBR\n");
    retval = CGPT_FAILED;
    goto done;
  }

  char buf[GUID_STRLEN];
  GuidToStr(&drive.pmbr.syslinux3.boot_guid, buf, sizeof(buf));

  int numEntries = GetNumberOfEntries(&drive);
  int i;
  for(i = 0; i < numEntries; i++) {
      GptEntry *entry = GetEntry(&drive.gpt, ANY_VALID, i);

      if (GuidEqual(&entry->unique, &drive.pmbr.syslinux3.boot_guid)) {
        params->partition = i + 1;
        retval = CGPT_OK;
        goto done;
      }
  }

  Error("Didn't find any boot partition\n");
  params->partition = 0;
  retval = CGPT_FAILED;

done:
  (void) DriveClose(&drive, 1);
  return retval;
}


int CgptBoot(CgptBootParams *params) {
  struct drive drive;
  int retval = 1;
  int gpt_retval= 0;
  int mode = O_RDONLY;

  if (params == NULL)
    return CGPT_FAILED;

  if (params->create_pmbr || params->partition || params->bootfile)
    mode = O_RDWR;

  if (CGPT_OK != DriveOpen(params->drive_name, &drive, 0, mode)) {
    return CGPT_FAILED;
  }

  if (CGPT_OK != ReadPMBR(&drive)) {
    Error("Unable to read PMBR\n");
    goto done;
  }

  if (params->create_pmbr) {
    InitPMBR(&drive);
    drive.pmbr.magic[0] = 0x1d;
    drive.pmbr.magic[1] = 0x9a;
  }

  if (params->partition) {
    if (GPT_SUCCESS != (gpt_retval = GptSanityCheck(&drive.gpt))) {
      Error("GptSanityCheck() returned %d: %s\n",
            gpt_retval, GptError(gpt_retval));
      goto done;
    }

    if (params->partition > GetNumberOfEntries(&drive)) {
      Error("invalid partition number: %d\n", params->partition);
      goto done;
    }

    uint32_t index = params->partition - 1;
    GptEntry *entry = GetEntry(&drive.gpt, ANY_VALID, index);
    memcpy(&drive.pmbr.syslinux3.boot_guid, &entry->unique, sizeof(Guid));
  }

  if (params->bootfile) {
    int fd = open(params->bootfile, O_RDONLY);
    if (fd < 0) {
      Error("Can't read %s: %s\n", params->bootfile, strerror(errno));
      goto done;
    }

    int n = read(fd, drive.pmbr.syslinux3.bootcode,
                 sizeof(drive.pmbr.syslinux3.bootcode));
    if (n < 1) {
      Error("problem reading %s: %s\n", params->bootfile, strerror(errno));
      close(fd);
      goto done;
    }

    close(fd);
  }

  char buf[GUID_STRLEN];
  GuidToStr(&drive.pmbr.syslinux3.boot_guid, buf, sizeof(buf));
  printf("%s\n", buf);

  // Write it all out, if needed.
  if (mode == O_RDONLY || CGPT_OK == WritePMBR(&drive))
    retval = 0;

done:
  (void) DriveClose(&drive, 1);
  return retval;
}
