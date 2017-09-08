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
#include <Library/DebugLib.h>
#include <Library/HobLib.h>

#include <Platform/MemoryMap.h>
#include <Platform/Pcie.h>

#define ARM_MEMORY_REGION(Base, Size) \
  { (Base), (Base), (Size), ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK }

#define ARM_DEVICE_REGION(Base, Size) \
  { (Base), (Base), (Size), ARM_MEMORY_REGION_ATTRIBUTE_DEVICE }

VOID
BuildMemoryTypeInformationHob (
  VOID
  );

STATIC ARM_MEMORY_REGION_DESCRIPTOR mVirtualMemoryTable[] = {
  // DDR - 2 GB
  ARM_MEMORY_REGION (SYNQUACER_SYSTEM_MEMORY_1_BASE,
                     SYNQUACER_SYSTEM_MEMORY_1_SZ),

  // DDR - 30 GB
  ARM_MEMORY_REGION (SYNQUACER_SYSTEM_MEMORY_2_BASE,
                     SYNQUACER_SYSTEM_MEMORY_2_SZ),

  // DDR - 32 GB
//  ARM_MEMORY_REGION (SYNQUACER_SYSTEM_MEMORY_3_BASE,
//                     SYNQUACER_SYSTEM_MEMORY_3_SZ),

  // Synquacer OnChip non-secure ROM
  ARM_MEMORY_REGION (SYNQUACER_NON_SECURE_ROM_BASE,
                     SYNQUACER_NON_SECURE_ROM_SZ),

  // Synquacer OnChip peripherals
  ARM_DEVICE_REGION (SYNQUACER_PERIPHERALS_BASE,
                     SYNQUACER_PERIPHERALS_SZ),

  // Synquacer OnChip non-secure SRAM
  ARM_MEMORY_REGION (SYNQUACER_NON_SECURE_SRAM_BASE,
                     SYNQUACER_NON_SECURE_SRAM_SZ),

  // Synquacer GIC-500
  ARM_DEVICE_REGION (SYNQUACER_GIC500_DIST_BASE, SYNQUACER_GIC500_DIST_SIZE),
  ARM_DEVICE_REGION (SYNQUACER_GIC500_RDIST_BASE, SYNQUACER_GIC500_RDIST_SIZE),

  // Synquacer eMMC(SDH30)
  ARM_DEVICE_REGION (SYNQUACER_EMMC_BASE, SYNQUACER_EMMC_BASE_SZ),

  // Synquacer EEPROM
  ARM_DEVICE_REGION (SYNQUACER_EEPROM_BASE, SYNQUACER_EEPROM_BASE_SZ),

  // Synquacer NETSEC
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

  { }
};

EFI_STATUS
EFIAPI
MemoryPeim (
  IN EFI_PHYSICAL_ADDRESS       UefiMemoryBase,
  IN UINT64                     UefiMemorySize
  )
{
  EFI_RESOURCE_ATTRIBUTE_TYPE   ResourceAttributes;
  RETURN_STATUS                 Status;

  ResourceAttributes =
      EFI_RESOURCE_ATTRIBUTE_PRESENT |
      EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_TESTED;

  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    ResourceAttributes,
    SYNQUACER_SYSTEM_MEMORY_1_BASE,
    SYNQUACER_SYSTEM_MEMORY_1_SZ);

  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    ResourceAttributes,
    SYNQUACER_SYSTEM_MEMORY_2_BASE,
    SYNQUACER_SYSTEM_MEMORY_2_SZ);

//  BuildResourceDescriptorHob (
//    EFI_RESOURCE_SYSTEM_MEMORY,
//    ResourceAttributes,
//    SYNQUACER_SYSTEM_MEMORY_3_BASE,
//    SYNQUACER_SYSTEM_MEMORY_3_SZ);

  Status = ArmConfigureMmu (mVirtualMemoryTable, NULL, NULL);
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
