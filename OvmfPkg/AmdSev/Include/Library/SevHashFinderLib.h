/** @file
  Validate a hash against that in the Sev Hash table

  Copyright (C) 2021 James Bottomley, IBM Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __SEV_HASH_FINDER_LIB_H__
#define __SEV_HASH_FINDER_LIB_H__

/**
  The Sev Hash table must be in encrypted memory and has the table
  and its entries described by

  <GUID>|UINT16 <len>|<data>

  With the whole table GUID being 9438d606-4f22-4cc9-b479-a793d411fd21

  The current possible table entries are for the kernel, the initrd
  and the cmdline:

  4de79437-abd2-427f-b835-d5b172d2045b  kernel
  44baf731-3a2f-4bd7-9af1-41e29169781d  initrd
  97d02dd8-bd20-4c94-aa78-e7714d36ab2a  cmdline

  The size of the entry is used to identify the hash, but the
  expectation is that it will be 32 bytes of SHA-256.
**/

#define SEV_HASH_TABLE_GUID \
  (GUID) { 0x9438d606, 0x4f22, 0x4cc9, { 0xb4, 0x79, 0xa7, 0x93, 0xd4, 0x11, 0xfd, 0x21 } }
#define SEV_KERNEL_HASH_GUID \
  (GUID) { 0x4de79437, 0xabd2, 0x427f, { 0xb8, 0x35, 0xd5, 0xb1, 0x72, 0xd2, 0x04, 0x5b } }
#define SEV_INITRD_HASH_GUID \
  (GUID) { 0x44baf731, 0x3a2f, 0x4bd7, { 0x9a, 0xf1, 0x41, 0xe2, 0x91, 0x69, 0x78, 0x1d } }
#define SEV_CMDLINE_HASH_GUID \
  (GUID) { 0x97d02dd8, 0xbd20, 0x4c94, { 0xaa, 0x78, 0xe7, 0x71, 0x4d, 0x36, 0xab, 0x2a } }

EFI_STATUS
EFIAPI
ValidateHashEntry (
  IN CONST GUID *Guid,
  IN CONST VOID *Buf,
  UINT32 BufSize
);

#endif
