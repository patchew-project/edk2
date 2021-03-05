/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Common.h"
#include "Gtt.h"
#include "VirtualGpu.h"

EFI_STATUS
GGTTGetEntry (
  IN  GVT_GOP_PRIVATE_DATA *Private,
  IN  UINT64 Index,
  OUT GTT_PTE_ENTRY *Entry
  )
{
  EFI_STATUS          Status = EFI_INVALID_PARAMETER;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PINTEL_VIRTUAL_GPU  VirtualGpu;

  if (Entry == NULL) {
    Status = EFI_INVALID_PARAMETER;
    GVT_DEBUG (EFI_D_ERROR,
      "%a invalid GGTT entry ptr %p at Index %x, status %d\n",
      __FUNCTION__, Entry, Index, Status
      );
    goto Done;
  }

  PciIo = Private->PciIo;
  VirtualGpu = (PINTEL_VIRTUAL_GPU)Private->VirtualGpu;

  if (Index >= VirtualGpu->VisibleGGTTOffset &&
       Index < VirtualGpu->VisibleGGTTOffset + VirtualGpu->VisibleGGTTSize) {
    Status = PciIo->Mem.Read (
                          PciIo,
                          EfiPciIoWidthUint64,
                          PCI_BAR_IDX0,
                          GTT_OFFSET + Index * GTT_ENTRY_SIZE,
                          1,
                          Entry
                          );
    if (EFI_ERROR (Status)) {
      Entry = 0;
      GVT_DEBUG (EFI_D_ERROR,
        "Failed to Get GGTT Entry index %lx, status %d\n", Index, Status
        );
    }
  } else if (Index >= VirtualGpu->InvisibleGGTTOffset &&
             Index < VirtualGpu->InvisibleGGTTOffset + VirtualGpu->InvisibleGGTTSize) {
    Status = EFI_UNSUPPORTED;
    GVT_DEBUG (EFI_D_ERROR,
      "Skip get GGTT index %lx for invisible GMADR\n", Index
      );
  } else {
    Status = EFI_OUT_OF_RESOURCES;
    GVT_DEBUG (EFI_D_ERROR,
      "Skip get GGTT index %lx out-of-range, balloon unsupported\n",
      Index
      );
  }

  GVT_DEBUG (EFI_D_VERBOSE, "Get GGTT Entry %lx at index %lx\n", *Entry, Index);

Done:

  return Status;
}

EFI_STATUS
GGTTSetEntry (
  IN GVT_GOP_PRIVATE_DATA *Private,
  IN UINT64 Index,
  IN GTT_PTE_ENTRY Entry
  )
{
  EFI_STATUS          Status = EFI_INVALID_PARAMETER;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PINTEL_VIRTUAL_GPU  VirtualGpu;

  PciIo = Private->PciIo;
  VirtualGpu = (PINTEL_VIRTUAL_GPU)Private->VirtualGpu;

  if (Index >= VirtualGpu->VisibleGGTTOffset &&
      Index < VirtualGpu->VisibleGGTTOffset + VirtualGpu->VisibleGGTTSize) {
    Status = PciIo->Mem.Write (
                          PciIo,
                          EfiPciIoWidthUint64,
                          PCI_BAR_IDX0,
                          GTT_OFFSET + Index * GTT_ENTRY_SIZE,
                          1,
                          &Entry
                          );
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "Failed to Set GGTT Entry %lx at index %lx, status %d\n",
        Entry, Index, Status
        );
    }
  } else if (Index >= VirtualGpu->InvisibleGGTTOffset &&
             Index < VirtualGpu->InvisibleGGTTOffset + VirtualGpu->InvisibleGGTTSize) {
    Status = EFI_UNSUPPORTED;
    GVT_DEBUG (EFI_D_ERROR,
      "Skip set GGTT index %lx for invisible GMADR\n", Index
      );
  } else {
    Status = EFI_OUT_OF_RESOURCES;
    GVT_DEBUG (EFI_D_ERROR,
      "Skip set GGTT index %lx out-of-range, balloon unsupported\n", Index
      );
  }

  GVT_DEBUG (EFI_D_VERBOSE, "Set GGTT Entry %lx at index %lx\n", Entry, Index);

  return Status;
}

EFI_STATUS
UpdateGGTT (
  IN GVT_GOP_PRIVATE_DATA *Private,
  IN EFI_PHYSICAL_ADDRESS GMAddr,
  IN EFI_PHYSICAL_ADDRESS SysAddr,
  IN UINTN                Pages
  )
{
  EFI_STATUS         Status = EFI_INVALID_PARAMETER;
  PINTEL_VIRTUAL_GPU VirtualGpu;
  UINTN              GttOffset, Index;
  GTT_PTE_ENTRY      Entry;

  if (!IS_ALIGNED(SysAddr, GTT_PAGE_SIZE)) {
    Status = EFI_INVALID_PARAMETER;
    GVT_DEBUG (EFI_D_ERROR,
      "Failed to update GGTT GMADR %lx, SysAddr %lx isn't aligned to 0x%lx, status %d\n",
      GMAddr, SysAddr, GTT_PAGE_SIZE, Status
      );
    goto Done;
  }

  VirtualGpu = (PINTEL_VIRTUAL_GPU)Private->VirtualGpu;
  GttOffset = (GMAddr - VirtualGpu->GpuMemAddr) >> GTT_PAGE_SHIFT;

  GVT_DEBUG (EFI_D_VERBOSE,
    "Update GGTT GMADR %lx, SysAddr %lx, Pages 0x%lx\n",
    GMAddr, SysAddr, Pages
    );
  for (Index = 0; Index < Pages; Index++) {
    Entry = SysAddr + Index * GTT_PAGE_SIZE;
    Entry |= (GTT_PAGE_PRESENT | GTT_PAGE_READ_WRITE);
    Entry |= (GTT_PAGE_PWT | GTT_PAGE_PCD);
    GGTTSetEntry (Private, GttOffset + Index, Entry);
  }

  Status = EFI_SUCCESS;

Done:
  return Status;
}
