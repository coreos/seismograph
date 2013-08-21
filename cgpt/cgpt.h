// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VBOOT_REFERENCE_UTILITY_CGPT_CGPT_H_
#define VBOOT_REFERENCE_UTILITY_CGPT_CGPT_H_

#include <fcntl.h>
#include <features.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "gpt.h"
#include "cgptlib.h"


struct legacy_partition {
  uint8_t  status;
  uint8_t  f_head;
  uint8_t  f_sect;
  uint8_t  f_cyl;
  uint8_t  type;
  uint8_t  l_head;
  uint8_t  l_sect;
  uint8_t  l_cyl;
  uint32_t f_lba;
  uint32_t num_sect;
} __attribute__((packed));


// syslinux uses this format:
struct pmbr {
  uint8_t                 bootcode[424];
  Guid                    boot_guid;
  uint32_t                disk_id;
  uint8_t                 magic[2];     // 0x1d, 0x9a
  struct legacy_partition part[4];
  uint8_t                 sig[2];       // 0x55, 0xaa
} __attribute__((packed));

void PMBRToStr(struct pmbr *pmbr, char *str, unsigned int buflen);
char *IsWholeDev(const char *basename);

// Handle to the drive storing the GPT.
struct drive {
  int fd;           /* file descriptor */
  uint64_t size;    /* total size (in bytes) */
  GptData gpt;
  struct pmbr pmbr;
};


/* mode should be O_RDONLY or O_RDWR */
int DriveOpen(const char *drive_path, struct drive *drive,
              off_t min_size, int mode);
int DriveClose(struct drive *drive, int update_as_needed);
int CheckValid(const struct drive *drive);

/* Constant global type values to compare against */
extern const Guid guid_chromeos_firmware;
extern const Guid guid_chromeos_kernel;
extern const Guid guid_chromeos_rootfs;
extern const Guid guid_linux_data;
extern const Guid guid_chromeos_reserved;
extern const Guid guid_efi;
extern const Guid guid_unused;

int ReadPMBR(struct drive *drive);
int WritePMBR(struct drive *drive);

/* Convert possibly unterminated UTF16 string to UTF8.
 * Caller must prepare enough space for UTF8, which could be up to
 * twice the byte length of UTF16 string plus the terminating '\0'.
 *
 * Return: CGPT_OK --- all character are converted successfully.
 *         CGPT_FAILED --- convert error, i.e. output buffer is too short.
 */
int UTF16ToUTF8(const uint16_t *utf16, unsigned int maxinput,
                uint8_t *utf8, unsigned int maxoutput);

/* Convert null-terminated UTF8 string to UTF16.
 * Caller must prepare enough space for UTF16, which is the byte length of UTF8
 * plus the terminating 0x0000.
 *
 * Return: CGPT_OK --- all character are converted successfully.
 *         CGPT_FAILED --- convert error, i.e. output buffer is too short.
 */
int UTF8ToUTF16(const uint8_t *utf8, uint16_t *utf16, unsigned int maxoutput);

/* Helper functions for supported GPT types. */
int ResolveType(const Guid *type, char *buf);
int SupportedType(const char *name, Guid *type);
void PrintTypes(void);
void EntryDetails(GptEntry *entry, uint32_t index, int raw);

uint32_t GetNumberOfEntries(const struct drive *drive);
GptEntry *GetEntry(GptData *gpt, int secondary, uint32_t entry_index);
void SetPriority(struct drive *drive, int secondary, uint32_t entry_index,
                 int priority);
int GetPriority(struct drive *drive, int secondary, uint32_t entry_index);
void SetTries(struct drive *drive, int secondary, uint32_t entry_index,
              int tries);
int GetTries(struct drive *drive, int secondary, uint32_t entry_index);
void SetSuccessful(struct drive *drive, int secondary, uint32_t entry_index,
                   int success);
int GetSuccessful(struct drive *drive, int secondary, uint32_t entry_index);

void SetRaw(struct drive *drive, int secondary, uint32_t entry_index,
           uint32_t raw);

void UpdateAllEntries(struct drive *drive);

uint8_t RepairHeader(GptData *gpt, const uint32_t valid_headers);
uint8_t RepairEntries(GptData *gpt, const uint32_t valid_entries);
void UpdateCrc(GptData *gpt);
int IsSynonymous(const GptHeader* a, const GptHeader* b);

int IsUnused(struct drive *drive, int secondary, uint32_t index);
int IsKernel(struct drive *drive, int secondary, uint32_t index);
int IsRoot(struct drive *drive, int secondary, uint32_t index);

// For usage and error messages.
extern const char* progname;
extern const char* command;
void Error(const char *format, ...);

// The code paths that require uuid_generate are not used currently in
// libcgpt-cc.a so using this method would create an unnecessary dependency
// on libuuid which then requires us to build it for 32-bit for the static
// post-installer. So, we just expose this function pointer which should be
// set to uuid_generate in case of the cgpt binary and can be null or some
// no-op method in case of ilbcgpt-cc.a.
extern void (*uuid_generator)(uint8_t* buffer);

// Command functions.
int cmd_show(int argc, char *argv[]);
int cmd_repair(int argc, char *argv[]);
int cmd_create(int argc, char *argv[]);
int cmd_add(int argc, char *argv[]);
int cmd_boot(int argc, char *argv[]);
int cmd_find(int argc, char *argv[]);
int cmd_prioritize(int argc, char *argv[]);
int cmd_legacy(int argc, char *argv[]);
int cmd_next(int argc, char *argv[]);

#define ARRAY_COUNT(array) (sizeof(array)/sizeof((array)[0]))
const char *GptError(int errnum);

// Size in chars of the GPT Entry's PartitionName field
#define GPT_PARTNAME_LEN 72

/* The standard "assert" macro goes away when NDEBUG is defined. This doesn't.
 */
#define require(A) do { \
  if (!(A)) { \
    fprintf(stderr, "condition (%s) failed at %s:%d\n", \
            #A, __FILE__, __LINE__); \
    exit(1); } \
  } while (0)

#endif  // VBOOT_REFERENCE_UTILITY_CGPT_CGPT_H_
