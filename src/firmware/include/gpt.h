/* Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Defines EFI related structure. See more details in EFI 2.3 spec.
 *
 * To download EFI standard, please visit UEFI homepage:
 *    http://www.uefi.org/
 */
#ifndef VBOOT_REFERENCE_CGPTLIB_GPT_H_
#define VBOOT_REFERENCE_CGPTLIB_GPT_H_
#include <stdint.h>

#define GPT_HEADER_SIGNATURE  "EFI PART"
#define GPT_HEADER_SIGNATURE2 "CHROMEOS"
#define GPT_HEADER_SIGNATURE_SIZE sizeof(GPT_HEADER_SIGNATURE)
#define GPT_HEADER_REVISION 0x00010000

/*
 * The first 3 numbers should be stored in network-endian format according to
 * the GUID RFC.  The UEFI spec appendix A claims they should be stored in
 * little-endian format.  But they need to be _displayed_ in network-endian
 * format, which is also how they're documented in the specs.
 *
 * Since what we have here are little-endian constants, they're byte-swapped
 * from the normal display order.
 */
#define GPT_ENT_TYPE_UNUSED \
	{{{0x00000000,0x0000,0x0000,0x00,0x00,{0x00,0x00,0x00,0x00,0x00,0x00}}}}
#define GPT_ENT_TYPE_EFI \
	{{{0xc12a7328,0xf81f,0x11d2,0xba,0x4b,{0x00,0xa0,0xc9,0x3e,0xc9,0x3b}}}}
#define GPT_ENT_TYPE_BIOS \
	{{{0x21686148,0x6449,0x6e6f,0x74,0x4e,{0x65,0x65,0x64,0x45,0x46,0x49}}}}
#define GPT_ENT_TYPE_CHROMEOS_FIRMWARE \
	{{{0xcab6e88e,0xabf3,0x4102,0xa0,0x7a,{0xd4,0xbb,0x9b,0xe3,0xc1,0xd3}}}}
#define GPT_ENT_TYPE_CHROMEOS_KERNEL \
	{{{0xfe3a2a5d,0x4f32,0x41a7,0xb7,0x25,{0xac,0xcc,0x32,0x85,0xa3,0x09}}}}
#define GPT_ENT_TYPE_CHROMEOS_ROOTFS \
	{{{0x3cb8e202,0x3b7e,0x47dd,0x8a,0x3c,{0x7f,0xf2,0xa1,0x3c,0xfc,0xec}}}}
#define GPT_ENT_TYPE_CHROMEOS_RESERVED \
	{{{0x2e0a753d,0x9e48,0x43b0,0x83,0x37,{0xb1,0x51,0x92,0xcb,0x1b,0x5e}}}}
#define GPT_ENT_TYPE_MSWIN_DATA \
	{{{0xebd0a0a2,0xb9e5,0x4433,0x87,0xc0,{0x68,0xb6,0xb7,0x26,0x99,0xc7}}}}

#define GPT_ENT_TYPE_LINUX_DATA \
	{{{0x0fc63daf,0x8483,0x4772,0x8e,0x79,{0x3d,0x69,0xd8,0x47,0x7d,0xe4}}}}
#define GPT_ENT_TYPE_LINUX_SWAP \
	{{{0x0657fd6d,0xa4ab,0x43c4,0x84,0xe5,{0x09,0x33,0xc8,0x4b,0x4f,0x4f}}}}
#define GPT_ENT_TYPE_LINUX_BOOT \
	{{{0xBC13C2FF,0x59E6,0x4262,0xa3,0x52,{0xb2,0x75,0xfd,0x6f,0x71,0x72}}}}
#define GPT_ENT_TYPE_LINUX_HOME \
	{{{0x933ac7e1,0x2eb4,0x4f13,0xb8,0x44,{0x0e,0x14,0xe2,0xae,0xf9,0x15}}}}
#define GPT_ENT_TYPE_LINUX_LVM \
	{{{0xe6d6d379,0xf507,0x44c2,0xa2,0x3c,{0x23,0x8f,0x2a,0x3d,0xf9,0x28}}}}
#define GPT_ENT_TYPE_LINUX_RAID \
	{{{0xa19d880f,0x05fc,0x4d3b,0xa0,0x06,{0x74,0x3f,0x0f,0x84,0x91,0x1e}}}}
#define GPT_ENT_TYPE_LINUX_RESERVED \
	{{{0x8da63339,0x0007,0x60c0,0xc4,0x36,{0x08,0x3a,0xc8,0x23,0x09,0x08}}}}

#define	GPT_ENT_TYPE_COREOS_ROOTFS	\
	{{{0x5dfbf5f4,0x2848,0x4bac,0xaa,0x5e,{0xd,0x9a,0x20,0xb7,0x45,0xa6}}}}
#define	GPT_ENT_TYPE_COREOS_RESIZE	\
	{{{0x3884dd41,0x8582,0x4404,0xb9,0xa8,{0xe9,0xb8,0x4f,0x2d,0xf5,0x0e}}}}
#define	GPT_ENT_TYPE_COREOS_RESERVED	\
	{{{0xc95dc21a,0xdf0e,0x4340,0x8d,0x7b,{0x26,0xcb,0xfa,0x9a,0x3,0xe0}}}}
#define	GPT_ENT_TYPE_COREOS_ROOT_RAID	\
	{{{0xbe9067b9,0xea49,0x4f15,0xb4,0xf6,{0xf3,0x6f,0x8c,0x9e,0x18,0x18}}}}

#define UUID_NODE_LEN 6
#define GUID_SIZE 16

/* GUID definition. Defined in appendix A of EFI standard. */
typedef struct {
	union {
		struct {
			uint32_t time_low;
			uint16_t time_mid;
			uint16_t time_high_and_version;
			uint8_t clock_seq_high_and_reserved;
			uint8_t clock_seq_low;
			uint8_t node[UUID_NODE_LEN];
		} Uuid;
		uint8_t raw[GUID_SIZE];
	} u;
} __attribute__((packed)) Guid;

#define GUID_EXPECTED_SIZE GUID_SIZE

/*
 * GPT header defines how many partitions exist on a drive and sectors managed.
 * For every drive device, there are 2 headers, primary and secondary.  Most of
 * fields are duplicated except my_lba and entries_lba.
 *
 * You may find more details in chapter 5 of EFI standard.
 */
typedef struct {
	char signature[8];
	uint32_t revision;
	uint32_t size;
	uint32_t header_crc32;
	uint32_t reserved_zero;
	uint64_t my_lba;
	uint64_t alternate_lba;
	uint64_t first_usable_lba;
	uint64_t last_usable_lba;
	Guid disk_uuid;
	uint64_t entries_lba;
	uint32_t number_of_entries;
	uint32_t size_of_entry;
	uint32_t entries_crc32;
	/* Remainder of sector is reserved and should be 0 */
} __attribute__((packed)) GptHeader;

#define GPTHEADER_EXPECTED_SIZE 92

/*
 * GPT partition entry defines the starting and ending LBAs of a partition.  It
 * also contains the unique GUID, type, and attribute bits.
 *
 * You may find more details in chapter 5 of EFI standard.
 */
typedef struct {
	Guid type;
	Guid unique;
	uint64_t starting_lba;
	uint64_t ending_lba;
	union {
		struct {
			uint16_t reserved[3];
			uint16_t gpt_att;
		} __attribute__((packed)) fields;
		uint64_t whole;
	} attrs;
	uint16_t name[36];  /* UTF-16 encoded partition name */
	/* Remainder of entry is reserved and should be 0 */
} __attribute__((packed)) GptEntry;

#define GPTENTRY_EXPECTED_SIZE 128

#endif  /* VBOOT_REFERENCE_CGPTLIB_GPT_H_ */
