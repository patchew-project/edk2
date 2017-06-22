/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*  Copyright (c) 2017, Cadence Design Systems, Inc. All rights reserved.
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

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS          14

#define GICD_SZ SIZE_64KB
#define GICR_SZ SIZE_128KB

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  UINTN                         Index = 0;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  ASSERT(VirtualMemoryMap != NULL);

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR*)AllocatePages(EFI_SIZE_TO_PAGES (sizeof(ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (VirtualMemoryTable == NULL) {
    return;
  }
  // SRAM
  VirtualMemoryTable[Index].PhysicalBase = PcdGet64(PcdSystemMemoryBase);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdSystemMemoryBase);
  VirtualMemoryTable[Index].Length       = PcdGet64(PcdSystemMemorySize);
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // CDNS UART
  VirtualMemoryTable[++Index].PhysicalBase = PcdGet64(PcdCspSerialBase);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdCspSerialBase);
  VirtualMemoryTable[Index].Length       = PcdGet32(PcdCspSerialSize);
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // UEFI ROM (Secure)
  VirtualMemoryTable[++Index].PhysicalBase = PcdGet64(PcdSecureFdBaseAddress);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdSecureFdBaseAddress);
  VirtualMemoryTable[Index].Length       = PcdGet32(PcdSecureFdSize);
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // UEFI ROM (Normal)
  VirtualMemoryTable[++Index].PhysicalBase = PcdGet64(PcdFdBaseAddress);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdFdBaseAddress);
  VirtualMemoryTable[Index].Length       = PcdGet32(PcdFdSize);
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // CSP SysRegs
  VirtualMemoryTable[++Index].PhysicalBase = PcdGet64(PcdCspSysRegBase);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdCspSysRegBase);
  VirtualMemoryTable[Index].Length       = PcdGet32(PcdCspSysRegSize);
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // GIC Distributor
  VirtualMemoryTable[++Index].PhysicalBase = PcdGet64(PcdGicDistributorBase);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdGicDistributorBase);
  VirtualMemoryTable[Index].Length       = GICD_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // GIC Redistributor
  VirtualMemoryTable[++Index].PhysicalBase = PcdGet64(PcdGicRedistributorsBase);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdGicRedistributorsBase);
  VirtualMemoryTable[Index].Length       = GICR_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // PCIe RP
  VirtualMemoryTable[++Index].PhysicalBase = PcdGet64(PcdPcieRootPortBaseAddress);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdPcieRootPortBaseAddress);
  VirtualMemoryTable[Index].Length       = SIZE_8MB;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  //
  // PCI Configuration Space (AXI region)
  //
  VirtualMemoryTable[++Index].PhysicalBase  = PcdGet64 (PcdPciConfigurationSpaceBaseAddress);
  VirtualMemoryTable[Index].VirtualBase     = PcdGet64 (PcdPciConfigurationSpaceBaseAddress);
  VirtualMemoryTable[Index].Length          = PcdGet64 (PcdPciConfigurationSpaceSize);
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase = 0;
  VirtualMemoryTable[Index].VirtualBase  = 0;
  VirtualMemoryTable[Index].Length       = 0;
  VirtualMemoryTable[Index].Attributes   = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT(Index < MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}
