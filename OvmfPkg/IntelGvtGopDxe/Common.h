/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __COMMON_H_

#include <Protocol/PciIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/GraphicsOutput.h>

#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>

#include <IndustryStandard/Pci.h>

#include "DebugHelper.h"

#define IS_ALIGNED(addr, size) (((UINTN) (addr) & (size - 1)) == 0)

typedef struct {
  UINT64                       Signature;
  EFI_HANDLE                   Handle;
  EFI_PCI_IO_PROTOCOL          *PciIo;
  UINT64                       OriginalPciAttr;
  EFI_GRAPHICS_OUTPUT_PROTOCOL GraphicsOutputProtocol;
  EFI_DEVICE_PATH_PROTOCOL     *GopDevPath;
  VOID                         *VirtualGpu;
} GVT_GOP_PRIVATE_DATA;

#define GVT_GOP_MAGIC                     SIGNATURE_64('G','V','T','G','V','G','O','P')
#define GVT_GOP_PRIVATE_DATA_FROM_THIS(a) CR(a, GVT_GOP_PRIVATE_DATA, GraphicsOutputProtocol, GVT_GOP_MAGIC)

#define __COMMON_H_
#endif //__COMMON_H_
