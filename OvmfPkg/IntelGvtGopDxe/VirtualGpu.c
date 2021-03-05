/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Common.h"
#include "VirtualGpu.h"
#include "GpuReg.h"
#include "Gtt.h"
#include "Display.h"
#include <Library/QemuFwCfgLib.h>

EFI_STATUS
IntelVirtualGpuActive (
  IN EFI_PCI_IO_PROTOCOL *PciIo
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  PCI_TYPE00 PciHdr = {0};
  UINT64     Magic = 0;
  UINT32     Version = 0;
  UINT16     VerMajor = 0, VerMinor = 0;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (PciHdr) / sizeof (UINT32),
                        &PciHdr
                        );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't read PCI config header, status %d\n", Status
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (!IS_PCI_DISPLAY (&PciHdr) || PciHdr.Hdr.VendorId != 0x8086) {
    GVT_DEBUG (EFI_D_VERBOSE,
      "Skip non Intel PCI Display [%04x:%04x] class:%x\n",
      PciHdr.Hdr.VendorId, PciHdr.Hdr.DeviceId, PciHdr.Hdr.ClassCode[2]
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Status = PciIo->Mem.Read (
                        PciIo,
                        EfiPciIoWidthUint64,
                        PCI_BAR_IDX0,
                        vgtif_reg(magic),
                        1,
                        &Magic
                        );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't read GVT magic from [%04x:%04x], status %d\n",
      PciHdr.Hdr.VendorId, PciHdr.Hdr.DeviceId, Status
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (Magic != VGT_MAGIC) {
    GVT_DEBUG (EFI_D_ERROR,
      "Read magic from [%04x:%04x], get %x expect %x\n",
      PciHdr.Hdr.VendorId, PciHdr.Hdr.DeviceId, Magic, VGT_MAGIC
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Status = PciIo->Mem.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        PCI_BAR_IDX0,
                        vgtif_reg(version_major),
                        1,
                        &Version
                        );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't read GVT version from [%04x:%04x], status %d\n",
      PciHdr.Hdr.VendorId, PciHdr.Hdr.DeviceId, Status
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  VerMajor = Version & 0xFFFF;
  VerMinor = (Version & 0xFFFF) >> 16;
  if (VerMajor < VGT_VERSION_MAJOR) {
    GVT_DEBUG (EFI_D_ERROR,
      "Check VGT interface version of [%04x:%04x], got %x.%x, expect %x.*\n",
      PciHdr.Hdr.VendorId, PciHdr.Hdr.DeviceId,
      VerMajor, VerMinor, VGT_VERSION_MAJOR
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  GVT_DEBUG (EFI_D_INFO,
    "Intel GVT-g virtual GPU [%04x:%04x] detected, version %x.%x\n",
    PciHdr.Hdr.VendorId, PciHdr.Hdr.DeviceId, VerMajor, VerMinor
    );
  Status = EFI_SUCCESS;

Done:
  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
IntelVirtualGpuInit (
  IN OUT GVT_GOP_PRIVATE_DATA *Private
  )
{
  EFI_STATUS           Status = EFI_UNSUPPORTED;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  PINTEL_VIRTUAL_GPU   VirtualGpu;
  FIRMWARE_CONFIG_ITEM FwCfgItem;
  UINTN                FwCfgSize;
  UINT8                Val8;
  UINT64               Val64;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  PciIo = Private->PciIo;
  VirtualGpu = (PINTEL_VIRTUAL_GPU)Private->VirtualGpu;
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_VENDOR_ID_OFFSET,
                        1,
                        &VirtualGpu->VendorId
                        );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't read PCI_VENDOR_ID_OFFSET, status %d\n", Status
      );
    goto Done;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_DEVICE_ID_OFFSET,
                        1,
                        &VirtualGpu->DeviceId
                        );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't read PCI_DEVICE_ID_OFFSET, status %d\n", Status
      );
    goto Done;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint64,
                        PCI_BASE_ADDRESSREG_OFFSET + PCI_BAR_IDX2 * 4,
                        1,
                        &Val64
                        );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR, "Can't get GMADR from BAR2, status %d\n", Status);
    goto Done;
  }

  if (Val64 & 0x1) {
    Status = EFI_OUT_OF_RESOURCES;
    GVT_DEBUG (EFI_D_ERROR, "BAR2 isn't memory space, status %d\n", Status);
    goto Done;
  }

  switch (Val64 >> 1 & 0x3) {
  case 0:
    VirtualGpu->GpuMemAddr = Val64 & 0xFFFFFFF0;
    GVT_DEBUG (EFI_D_VERBOSE, "BAR2 has 32-bit access space\n");
    break;
  case 2:
    VirtualGpu->GpuMemAddr = Val64 & ~0xF;
    GVT_DEBUG (EFI_D_VERBOSE, "BAR2 has 64-bit access space\n");
    break;
  default:
    Status = EFI_OUT_OF_RESOURCES;
    GVT_DEBUG (EFI_D_ERROR,
      "BAR2 has unknown access space, status %d\n", Status
      );
    goto Done;
    break;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_REG_MSAC,
                        1,
                        &Val8
                        );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't get MSAC from %x, status %d\n", PCI_REG_MSAC, Status
      );
    goto Done;
  }

  Val8 &= 0x1F;
  if (Val8 & 0x10) {
    VirtualGpu->GpuMemSizeM = 4096;
  } else {
    Val8 &= 0xF;
    if (Val8 & 0x8) {
      VirtualGpu->GpuMemSizeM = 2048;
    } else {
      Val8 &= 0x7;
      if (Val8 & 0x4) {
        VirtualGpu->GpuMemSizeM = 1024;
      } else {
        Val8 &= 0x3;
        if (Val8 & 0x2) {
          VirtualGpu->GpuMemSizeM = 512;
        } else {
          if (Val8 & 0x1) {
            VirtualGpu->GpuMemSizeM = 256;
          } else {
            VirtualGpu->GpuMemSizeM = 128;
          }
        }
      }
    }
  }

  Status = QemuFwCfgFindFile ("etc/igd-opregion", &FwCfgItem, &FwCfgSize);
  if (Status == EFI_SUCCESS && FwCfgSize == OPREGION_SIZE) {
    // OpRegion must sit below 4 GB
    VirtualGpu->OpRegion = SIZE_4GB - 1;
    Status = gBS->AllocatePages (
                AllocateMaxAddress,
                EfiReservedMemoryType,
                EFI_SIZE_TO_PAGES (OPREGION_SIZE),
                &VirtualGpu->OpRegion
                );
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "Fail to allocate %d pages size %lx for OpRegion, status %d\n",
        EFI_SIZE_TO_PAGES (OPREGION_SIZE), OPREGION_SIZE, Status
        );
      goto Done;
    }
    QemuFwCfgSelectItem (FwCfgItem);
    QemuFwCfgReadBytes (FwCfgSize, (VOID*)VirtualGpu->OpRegion);
    Status = PciIo->Pci.Write (
                          PciIo,
                          EfiPciIoWidthUint32,
                          PCI_REG_ASLS,
                          1,
                          (UINT32*)&VirtualGpu->OpRegion
                          );
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "Fail to write OpRegion %p to PCI config offset 0x%x, status %d\n",
        VirtualGpu->OpRegion, PCI_REG_ASLS, Status
        );
      goto Done;
    } else {
      GVT_DEBUG (EFI_D_INFO,
        "OpRegion %p is set to PCI config offset 0x%x\n",
        VirtualGpu->OpRegion, PCI_REG_ASLS
        );
    }
  } else {
    GVT_DEBUG (EFI_D_VERBOSE,
      "Not igd-opregion found in QEMU firmware config\n"
      );
  }

  RegRead32 (Private,
    vgtif_reg(avail_rs.mappable_gmadr.base), &VirtualGpu->VisibleOffset);
  RegRead32 (Private,
    vgtif_reg(avail_rs.mappable_gmadr.size), &VirtualGpu->VisibleSize);
  RegRead32 (Private,
    vgtif_reg(avail_rs.nonmappable_gmadr.base), &VirtualGpu->InvisibleOffset);
  RegRead32 (Private,
    vgtif_reg(avail_rs.nonmappable_gmadr.size), &VirtualGpu->InvisibleSize);
  VirtualGpu->VisibleGGTTOffset = VirtualGpu->VisibleOffset >> GTT_PAGE_SHIFT;
  VirtualGpu->VisibleGGTTSize = VirtualGpu->VisibleSize >> GTT_PAGE_SHIFT;
  VirtualGpu->InvisibleGGTTOffset = VirtualGpu->InvisibleOffset >> GTT_PAGE_SHIFT;
  VirtualGpu->InvisibleGGTTSize = VirtualGpu->InvisibleSize >> GTT_PAGE_SHIFT;

  GVT_DEBUG (
    EFI_D_INFO,
    "GMADR [0x%lx - 0x%lx], size %d MB\n",
    VirtualGpu->GpuMemAddr,
    VirtualGpu->GpuMemAddr + VirtualGpu->GpuMemSizeM * 0x100000,
    VirtualGpu->GpuMemSizeM
    );
  GVT_DEBUG (
    EFI_D_INFO,
    "visible offset [0x%x - 0x%x] size %d KB, GGTT range [%x - %x]\n",
    VirtualGpu->VisibleOffset,
    VirtualGpu->VisibleOffset + VirtualGpu->VisibleSize,
    VirtualGpu->VisibleSize / 0x400,
    VirtualGpu->VisibleGGTTOffset,
    VirtualGpu->VisibleGGTTOffset + VirtualGpu->VisibleGGTTSize
    );
  GVT_DEBUG (
    EFI_D_INFO,
    "invisible offset [0x%x - 0x%x] size %d KB, GGTT range [%x - %x]\n",
    VirtualGpu->InvisibleOffset,
    VirtualGpu->InvisibleOffset + VirtualGpu->InvisibleSize,
    VirtualGpu->InvisibleSize / 0x400,
    VirtualGpu->InvisibleGGTTOffset,
    VirtualGpu->InvisibleGGTTOffset + VirtualGpu->InvisibleGGTTSize
    );

  Status = IntelVirtualGpuDisplayInit (Private);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Fail to initialize display, status %d\n", Status
      );
    goto Done;
  }

  Status = IntelVirtualGpuSetMode (&Private->GraphicsOutputProtocol, 0);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Fail to set init display mode, status %d\n", Status
      );
    goto Done;
  }

  Status = IntelVirtualGpuNotifyDisplayReady (Private, TRUE);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Fail to notify display ready, status %d\n", Status
      );
    goto Done;
  }

  // Flush all reg after DisplayReady
  Status = IntelVirtualGpuEnableDisplay (
             Private,
             0,
             TRUE
             );

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
IntelVirtualGpuClean (
  IN OUT GVT_GOP_PRIVATE_DATA *Private
  )
{
  EFI_STATUS         Status = EFI_INVALID_PARAMETER;
  PINTEL_VIRTUAL_GPU VirtualGpu;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  Status = IntelVirtualGpuDisplayClean (Private);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR, "Fail to clean display, status %d\n", Status);
    goto Done;
  }

  VirtualGpu = (PINTEL_VIRTUAL_GPU)Private->VirtualGpu;
  if (VirtualGpu->OpRegion) {
    Status = gBS->FreePages (
                    VirtualGpu->OpRegion,
                    EFI_SIZE_TO_PAGES (OPREGION_SIZE)
                    );
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "FreePages failed for OpRegion, pages %d, size %d, status %d\n",
        EFI_SIZE_TO_PAGES (OPREGION_SIZE), OPREGION_SIZE, Status
        );
        goto Done;
    }
    Status = EFI_SUCCESS;
  }

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}
