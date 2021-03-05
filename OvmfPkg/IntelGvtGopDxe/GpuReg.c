/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "GpuReg.h"

EFI_STATUS
RegRead32 (
  IN  GVT_GOP_PRIVATE_DATA *Private,
  IN  UINT32 Offset,
  OUT UINT32 *ValuePtr
  )
{
  EFI_STATUS Status = EFI_INVALID_PARAMETER;

  if (Offset < MMIO_SIZE && ValuePtr != NULL) {
    Status = Private->PciIo->Mem.Read (
                          Private->PciIo,
                          EfiPciIoWidthUint32,
                          PCI_BAR_IDX0,
                          Offset,
                          1,
                          ValuePtr
                          );
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR, "%a reg %x, value %x, status %d\n",
        __FUNCTION__, Offset, *ValuePtr, Status
        );
    } else {
      Status = EFI_SUCCESS;
      GVT_DEBUG (EFI_D_VERBOSE, "%a reg %x, value %x, status %d\n",
        __FUNCTION__, Offset, *ValuePtr, Status
        );
    }
  } else {
    Status = EFI_INVALID_PARAMETER;
    GVT_DEBUG (EFI_D_ERROR,
      "%a invalid reg %x or ValuePtr %p, status %d\n",
      __FUNCTION__, Offset, ValuePtr, Status
      );
    goto Done;
  }

Done:
  return Status;
}

EFI_STATUS
RegWrite32 (
  IN GVT_GOP_PRIVATE_DATA *Private,
  IN UINT32 Offset,
  IN UINT32 Value
  )
{
  EFI_STATUS Status = EFI_INVALID_PARAMETER;

  if (Offset < MMIO_SIZE) {
    Status = Private->PciIo->Mem.Write (
                          Private->PciIo,
                          EfiPciIoWidthUint32,
                          PCI_BAR_IDX0,
                          Offset,
                          1,
                          &Value
                          );
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR, "%a reg %x, value %x, status %d\n",
        __FUNCTION__, Offset, Value, Status
        );
    } else {
      Status = EFI_SUCCESS;
      GVT_DEBUG (EFI_D_VERBOSE, "%a reg %x, value %x, status %d\n",
        __FUNCTION__, Offset, Value, Status
        );
    }
  } else {
    Status = EFI_INVALID_PARAMETER;
    GVT_DEBUG (EFI_D_ERROR, "%a invalid reg %x, status %d\n",
      __FUNCTION__, Offset, Status
      );
    goto Done;
  }

Done:
  return Status;
}
