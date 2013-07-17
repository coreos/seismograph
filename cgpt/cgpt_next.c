// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "cgpt.h"
#include "cgptlib_internal.h"
#include "vboot_host.h"

#define BUFSIZE 1024

char next_file_name[BUFSIZE];
int next_priority, next_index;

static int do_search(CgptNextParams *params, char *fileName) {
  struct drive drive;
  uint32_t max_part;
  int gpt_retval;
  int priority, tries, successful;
  int i;

  if (CGPT_OK != DriveOpen(fileName, &drive, O_RDONLY))
    return CGPT_FAILED;

  if (GPT_SUCCESS != (gpt_retval = GptSanityCheck(&drive.gpt))) {
    Error("GptSanityCheck() returned %d: %s\n",
          gpt_retval, GptError(gpt_retval));
    return CGPT_FAILED;
  }

  max_part = GetNumberOfEntries(&drive);

  for (i = 0; i < max_part; i++) {
    if (!IsRoot(&drive, PRIMARY, i))
      continue;

    priority = GetPriority(&drive, PRIMARY, i);
    tries = GetTries(&drive, PRIMARY, i);
    successful = GetSuccessful(&drive, PRIMARY, i);

    if (next_index == -1 || ((priority > next_priority) && (successful || tries))) {
      strncpy(next_file_name, fileName, BUFSIZE);
      next_priority = priority;
      next_index = i;
    }
  }

  return DriveClose(&drive, 0);
}

#define PROC_PARTITIONS "/proc/partitions"

// This scans all the physical devices it can find, looking for a match. It
// returns true if any matches were found, false otherwise.
static int scan_real_devs(CgptNextParams *params) {
  int found = 0;
  char line[BUFSIZE];
  char partname[128];                   // max size for /proc/partition lines?
  FILE *fp;
  char *pathname;

  fp = fopen(PROC_PARTITIONS, "r");
  if (!fp) {
    perror("can't read " PROC_PARTITIONS);
    return found;
  }

  while (fgets(line, sizeof(line), fp)) {
    int ma, mi;
    long long unsigned int sz;

    if (sscanf(line, " %d %d %llu %127[^\n ]", &ma, &mi, &sz, partname) != 4)
      continue;

    if ((pathname = IsWholeDev(partname))) {
      do_search(params, pathname);
    }
  }

  fclose(fp);
  return found;
}

int CgptNext(CgptNextParams *params) {
  struct drive drive;
  GptEntry *entry;
  char tmp[64];
  int tries;
  next_index = -1;
  int gpt_retval;

  if (params == NULL)
    return CGPT_FAILED;

  scan_real_devs(params);

  if (strlen(next_file_name) < 0) {
    return CGPT_FAILED;
  }

  if (DriveOpen(next_file_name, &drive, O_RDWR) == CGPT_OK) {
    if (GPT_SUCCESS != (gpt_retval = GptSanityCheck(&drive.gpt))) {
      Error("GptSanityCheck() returned %d: %s\n",
            gpt_retval, GptError(gpt_retval));
      return CGPT_FAILED;
    }

    // Decrement tries if we selected on that criteria
    tries = GetTries(&drive, PRIMARY, next_index);
    if (tries > 0) {
      tries--;
    }
    SetTries(&drive, PRIMARY, next_index, tries);

    // Print out the next disk to go!
    entry = GetEntry(&drive.gpt, ANY_VALID, next_index);
    GuidToStr(&entry->unique, tmp, sizeof(tmp));
    printf("%s\n", tmp);

    // Write it all out
    UpdateAllEntries(&drive);
    return DriveClose(&drive, 1);
  }

  return 1;
}
