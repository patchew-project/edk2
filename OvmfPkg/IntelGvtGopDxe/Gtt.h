/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __GTT_H_
#define __GTT_H_

#include "Common.h"

typedef UINT64 GTT_PTE_ENTRY;

#define GTT_OFFSET          0x800000
#define GTT_SIZE            0x800000
#define GTT_ENTRY_SIZE      sizeof(GTT_PTE_ENTRY)
#define GTT_ENTRY_NUM       (GTT_SIZE / GTT_ENTRY_SIZE)
#define GTT_PAGE_SHIFT      12
#define GTT_PAGE_SIZE       (1UL << GTT_PAGE_SHIFT)
#define GTT_PAGE_MASK       (~(GTT_PAGE_SIZE-1))
#define GTT_PAGE_PRESENT    0x01
#define GTT_PAGE_READ_WRITE 0x02
#define GTT_PAGE_PWT        0x08
#define GTT_PAGE_PCD        0x10

EFI_STATUS
GGTTGetEntry (
  IN  GVT_GOP_PRIVATE_DATA *Private,
  IN  UINT64 Index,
  OUT GTT_PTE_ENTRY *Entry
  );

EFI_STATUS
GGTTSetEntry (
  IN GVT_GOP_PRIVATE_DATA *Private,
  IN UINT64 Index,
  IN GTT_PTE_ENTRY Entry
  );

EFI_STATUS
UpdateGGTT (
  IN GVT_GOP_PRIVATE_DATA *Private,
  IN EFI_PHYSICAL_ADDRESS GMAddr,
  IN EFI_PHYSICAL_ADDRESS SysAddr,
  IN UINTN                Pages
  );

#endif //__GTT_H_
