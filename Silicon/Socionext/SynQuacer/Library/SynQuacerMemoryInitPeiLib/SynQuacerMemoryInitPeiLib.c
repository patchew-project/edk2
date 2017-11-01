/** @file
*
*  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
*  Copyright (c) 2017, Linaro, Ltd. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <PiPei.h>

#include <Library/ArmLib.h>
#include <Library/ArmMmuLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>

#include <Platform/MemoryMap.h>
#include <Platform/Pcie.h>

#include <Ppi/DramInfo.h>

#define ARM_MEMORY_REGION(Base, Size) \
  { (Base), (Base), (Size), ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK }

#define ARM_UNCACHED_REGION(Base, Size) \
  { (Base), (Base), (Size), ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED }

#define ARM_DEVICE_REGION(Base, Size) \
  { (Base), (Base), (Size), ARM_MEMORY_REGION_ATTRIBUTE_DEVICE }

VOID
BuildMemoryTypeInformationHob (
  VOID
  );

STATIC CONST EFI_RESOURCE_ATTRIBUTE_TYPE mDramResourceAttributes =
  EFI_RESOURCE_ATTRIBUTE_PRESENT |
  EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
  EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
  EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
  EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
  EFI_RESOURCE_ATTRIBUTE_TESTED;

STATIC CONST ARM_MEMORY_REGION_DESCRIPTOR mVirtualMemoryTable[] = {
  // Memory mapped SPI NOR flash
  // Mapped with device attributes for performance (!)
  ARM_DEVICE_REGION (FixedPcdGet64 (PcdFdBaseAddress),
                     FixedPcdGet32 (PcdFdSize)),

  // Memory mapped SPI NOR flash - XIP region
  // Sub-region of the preceding one - supersede with normal-nc attributes
  ARM_UNCACHED_REGION (FixedPcdGet64 (PcdFvBaseAddress),
                       FixedPcdGet32 (PcdFvSize)),

  // SynQuacer OnChip peripherals
  ARM_DEVICE_REGION (SYNQUACER_PERIPHERALS_BASE,
                     SYNQUACER_PERIPHERALS_SZ),

  // SynQuacer OnChip non-secure SRAM
  ARM_UNCACHED_REGION (SYNQUACER_NON_SECURE_SRAM_BASE,
                       SYNQUACER_NON_SECURE_SRAM_SZ),

  // SynQuacer GIC-500
  ARM_DEVICE_REGION (SYNQUACER_GIC500_DIST_BASE, SYNQUACER_GIC500_DIST_SIZE),
  ARM_DEVICE_REGION (SYNQUACER_GIC500_RDIST_BASE, SYNQUACER_GIC500_RDIST_SIZE),

  // SynQuacer eMMC(SDH30)
  ARM_DEVICE_REGION (SYNQUACER_EMMC_BASE, SYNQUACER_EMMC_BASE_SZ),

  // SynQuacer EEPROM - could point to NOR flash as well
  ARM_DEVICE_REGION (FixedPcdGet32 (PcdNetsecEepromBase),
                     SYNQUACER_EEPROM_BASE_SZ),

  // SynQuacer NETSEC
  ARM_DEVICE_REGION (SYNQUACER_NETSEC_BASE, SYNQUACER_NETSEC_BASE_SZ),

  // PCIe control registers
  ARM_DEVICE_REGION (SYNQUACER_PCIE_BASE, SYNQUACER_PCIE_SIZE),

  // PCIe config space
  ARM_DEVICE_REGION (SYNQUACER_PCI_SEG0_CONFIG_BASE,
                     SYNQUACER_PCI_SEG0_CONFIG_SIZE),
  ARM_DEVICE_REGION (SYNQUACER_PCI_SEG1_CONFIG_BASE,
                     SYNQUACER_PCI_SEG1_CONFIG_SIZE),

  // PCIe I/O space
  ARM_DEVICE_REGION (SYNQUACER_PCI_SEG0_PORTIO_MEMBASE,
                     SYNQUACER_PCI_SEG0_PORTIO_MEMSIZE),
  ARM_DEVICE_REGION (SYNQUACER_PCI_SEG1_PORTIO_MEMBASE,
                     SYNQUACER_PCI_SEG1_PORTIO_MEMSIZE),
};

STATIC
EFI_STATUS
DeclareDram (
  OUT ARM_MEMORY_REGION_DESCRIPTOR    **VirtualMemoryTable
  )
{
  SYNQUACER_DRAM_INFO_PPI       *DramInfo;
  EFI_STATUS                    Status;
  UINTN                         Idx;
  UINTN                         RegionCount;
  UINT64                        Base;
  UINT64                        Size;
  ARM_MEMORY_REGION_DESCRIPTOR  *DramDescriptor;

  Status = PeiServicesLocatePpi (&gSynQuacerDramInfoPpiGuid, 0, NULL,
             (VOID **)&DramInfo);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = DramInfo->GetRegionCount (&RegionCount);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *VirtualMemoryTable = AllocatePool (sizeof (mVirtualMemoryTable) +
                                      (RegionCount + 1) *
                                      sizeof (ARM_MEMORY_REGION_DESCRIPTOR));
  if (*VirtualMemoryTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (*VirtualMemoryTable, mVirtualMemoryTable,
    sizeof (mVirtualMemoryTable));

  DramDescriptor = *VirtualMemoryTable + ARRAY_SIZE (mVirtualMemoryTable);

  for (Idx = 0; Idx < RegionCount; Idx++, DramDescriptor++) {
    Status = DramInfo->GetRegion (Idx, &Base, &Size);
    ASSERT_EFI_ERROR (Status);

    BuildResourceDescriptorHob (EFI_RESOURCE_SYSTEM_MEMORY,
      mDramResourceAttributes, Base, Size);

    DramDescriptor->PhysicalBase = Base;
    DramDescriptor->VirtualBase  = Base;
    DramDescriptor->Length       = Size;
    DramDescriptor->Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
  }

  DramDescriptor->PhysicalBase = 0;
  DramDescriptor->VirtualBase  = 0;
  DramDescriptor->Length       = 0;
  DramDescriptor->Attributes   = 0;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MemoryPeim (
  IN EFI_PHYSICAL_ADDRESS       UefiMemoryBase,
  IN UINT64                     UefiMemorySize
  )
{
  EFI_STATUS                    Status;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  Status = DeclareDram (&VirtualMemoryTable);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ArmConfigureMmu (VirtualMemoryTable, NULL, NULL);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (FeaturePcdGet (PcdPrePiProduceMemoryTypeInformationHob)) {
    // Optional feature that helps prevent EFI memory map fragmentation.
    BuildMemoryTypeInformationHob ();
  }
  return EFI_SUCCESS;
}
