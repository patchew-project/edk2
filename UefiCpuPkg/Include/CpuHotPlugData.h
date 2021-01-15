/** @file
Definition for a structure sharing information for CPU hot plug.

Copyright (c) 2013 - 2015, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2021, Oracle Corporation.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _CPU_HOT_PLUG_DATA_H_
#define _CPU_HOT_PLUG_DATA_H_

#define  CPU_HOT_PLUG_DATA_REVISION_1      0x00000001

typedef struct {
  UINT32    Revision;          // Used for version identification for this structure
  UINT32    ArrayLength;       // The entries number of the following ApicId array and SmBase array
  //
  // Data required for SMBASE relocation
  //
  UINT64    *ApicId;           // Pointer to ApicId array
  UINTN     *SmBase;           // Pointer to SmBase array
  UINT32    Reserved;
  UINT32    SmrrBase;
  UINT32    SmrrSize;
} CPU_HOT_PLUG_DATA;

typedef
VOID
(EFIAPI *CPU_HOT_EJECT_FN)(
  IN UINTN  ProcessorNum
  );

#define CPU_EJECT_INVALID	(MAX_UINT64)
#define CPU_EJECT_WORKER	(MAX_UINT64-1)

#define  CPU_HOT_EJECT_DATA_REVISION_1         0x00000001

typedef struct {
  UINT32           Revision;          // Used for version identification for this structure
  UINT32           ArrayLength;       // The entries number of the following ApicId array and SmBase array

  UINT64           *ApicIdMap;        // Pointer to CpuIndex->ApicId map. Holds APIC IDs for pending ejects
  CPU_HOT_EJECT_FN Handler;           // Handler for CPU ejection
  UINT64           Reserved;
} CPU_HOT_EJECT_DATA;

#endif
