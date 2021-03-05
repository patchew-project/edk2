/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __VIRTUALGPU_H_
#define __VIRTUALGPU_H_

#include <Display.h>

#define PCI_REG_MSAC 0x62
#define PCI_REG_ASLS 0xFC

#define OPREGION_SIZE SIZE_8KB

typedef struct _INTEL_VIRTUAL_GPU {
  UINT16                    VendorId;
  UINT16                    DeviceId;
  EFI_PHYSICAL_ADDRESS      OpRegion;
  EFI_PHYSICAL_ADDRESS      GpuMemAddr;
  UINT32                    GpuMemSizeM;
  UINT32                    VisibleOffset;
  UINT32                    VisibleSize;
  UINT32                    VisibleGGTTOffset;
  UINT32                    VisibleGGTTSize;
  UINT32                    InvisibleOffset;
  UINT32                    InvisibleSize;
  UINT32                    InvisibleGGTTOffset;
  UINT32                    InvisibleGGTTSize;
  INTEL_VIRTUAL_GPU_DISPLAY Display;
} INTEL_VIRTUAL_GPU, *PINTEL_VIRTUAL_GPU;

EFI_STATUS
IntelVirtualGpuActive (
  IN EFI_PCI_IO_PROTOCOL *PciIo
  );

EFI_STATUS
IntelVirtualGpuInit (
  IN OUT GVT_GOP_PRIVATE_DATA *Private
  );

EFI_STATUS
IntelVirtualGpuClean (
  IN OUT GVT_GOP_PRIVATE_DATA *Private
  );

#endif //__VIRTUALGPU_H_
