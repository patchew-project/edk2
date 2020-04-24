/** @file
*
*  Copyright (c) 2018 - 2020, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <N1SdpPlatform.h>

// The total number of descriptors, including the final "end-of-table" descriptor.
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS 9

/**
  Returns the Virtual Memory Map of the platform.

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU
  on your platform.

  @param[out] VirtualMemoryMap Array of ARM_MEMORY_REGION_DESCRIPTOR describing
                               a Physical-to-Virtual Memory mapping. This array
                               must be ended by a zero-filled entry.
**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR **VirtualMemoryMap
  )
{
  UINTN                         Index = 0;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;
  EFI_RESOURCE_ATTRIBUTE_TYPE   ResourceAttributes;
  N1SDP_PLAT_INFO               *PlatInfo;
  UINT64                        DramBlock2Size;

  PlatInfo = (N1SDP_PLAT_INFO *)N1SDP_PLAT_INFO_STRUCT_BASE;
  DramBlock2Size = ((UINT64)(PlatInfo->LocalDdrSize -
                    (N1SDP_DRAM_BLOCK1_SIZE / SIZE_1GB)) * (UINT64)SIZE_1GB);

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
    FixedPcdGet64 (PcdDramBlock2Base),
    DramBlock2Size);

  ASSERT (VirtualMemoryMap != NULL);
  Index = 0;

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR *)AllocatePages
                       (EFI_SIZE_TO_PAGES (sizeof (ARM_MEMORY_REGION_DESCRIPTOR) *
                        MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (VirtualMemoryTable == NULL) {
    return;
  }

  // SubSystem Peripherals - Generic Watchdog
  VirtualMemoryTable[Index].PhysicalBase    = N1SDP_GENERIC_WDOG_BASE;
  VirtualMemoryTable[Index].VirtualBase     = N1SDP_GENERIC_WDOG_BASE;
  VirtualMemoryTable[Index].Length          = N1SDP_GENERIC_WDOG_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // SubSystem Peripherals - GIC-600
  VirtualMemoryTable[++Index].PhysicalBase  = N1SDP_GIC_BASE;
  VirtualMemoryTable[Index].VirtualBase     = N1SDP_GIC_BASE;
  VirtualMemoryTable[Index].Length          = N1SDP_GIC_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // SubSystem Peripherals - GICR-600
  VirtualMemoryTable[++Index].PhysicalBase  = N1SDP_GICR_BASE;
  VirtualMemoryTable[Index].VirtualBase     = N1SDP_GICR_BASE;
  VirtualMemoryTable[Index].Length          = N1SDP_GICR_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // OnChip non-secure SRAM
  VirtualMemoryTable[++Index].PhysicalBase  = N1SDP_NON_SECURE_SRAM_BASE;
  VirtualMemoryTable[Index].VirtualBase     = N1SDP_NON_SECURE_SRAM_BASE;
  VirtualMemoryTable[Index].Length          = N1SDP_NON_SECURE_SRAM_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // SubSystem Pheripherals - UART0
  VirtualMemoryTable[++Index].PhysicalBase  = N1SDP_UART0_BASE;
  VirtualMemoryTable[Index].VirtualBase     = N1SDP_UART0_BASE;
  VirtualMemoryTable[Index].Length          = N1SDP_UART0_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // DDR Primary (2GB)
  VirtualMemoryTable[++Index].PhysicalBase  = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].VirtualBase     = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].Length          = PcdGet64 (PcdSystemMemorySize);
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // DDR Secondary
  VirtualMemoryTable[++Index].PhysicalBase  = PcdGet64 (PcdDramBlock2Base);
  VirtualMemoryTable[Index].VirtualBase     = PcdGet64 (PcdDramBlock2Base);
  VirtualMemoryTable[Index].Length          = DramBlock2Size;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // Expansion Peripherals
  VirtualMemoryTable[++Index].PhysicalBase  = N1SDP_EXP_PERIPH_BASE0;
  VirtualMemoryTable[Index].VirtualBase     = N1SDP_EXP_PERIPH_BASE0;
  VirtualMemoryTable[Index].Length          = N1SDP_EXP_PERIPH_BASE0_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase  = 0;
  VirtualMemoryTable[Index].VirtualBase     = 0;
  VirtualMemoryTable[Index].Length          = 0;
  VirtualMemoryTable[Index].Attributes      = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT((Index) < MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);
  DEBUG ((DEBUG_INIT, "Virtual Memory Table setup complete.\n"));

  *VirtualMemoryMap = VirtualMemoryTable;
}
