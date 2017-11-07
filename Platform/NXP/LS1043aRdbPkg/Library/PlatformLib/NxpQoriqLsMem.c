/** NxpQoriqLsMem.c
*
*  Board memory specific Library.
*
*  Based on BeagleBoardPkg/Library/BeagleBoardLib/BeagleBoardMem.c
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*  Copyright (c) 2016, Freescale Semiconductor, Inc. All rights reserved.
*  Copyright 2017 NXP
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>

#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS          25

#define CCSR_BASE_ADDR            FixedPcdGet64 (PcdCcsrBaseAddr)
#define CCSR_SIZE                 FixedPcdGet64 (PcdCcsrSize)
#define IFC_REGION1_BASE_ADDR     FixedPcdGet64 (PcdIfcRegion1BaseAddr)
#define IFC_REGION1_SIZE          FixedPcdGet64 (PcdIfcRegion1Size)
#define IFC_REGION2_BASE_ADDR     FixedPcdGet64 (PcdIfcRegion2BaseAddr)
#define IFC_REGION2_SIZE          FixedPcdGet64 (PcdIfcRegion2Size)
#define QMAN_SWP_BASE_ADDR        FixedPcdGet64 (PcdQmanSwpBaseAddr)
#define QMAN_SWP_SIZE             FixedPcdGet64 (PcdQmanSwpSize)
#define BMAN_SWP_BASE_ADDR        FixedPcdGet64 (PcdBmanSwpBaseAddr)
#define BMAN_SWP_SIZE             FixedPcdGet64 (PcdBmanSwpSize)
#define PCI_EXP1_BASE_ADDR        FixedPcdGet64 (PcdPciExp1BaseAddr)
#define PCI_EXP1_BASE_SIZE        FixedPcdGet64 (PcdPciExp1BaseSize)
#define PCI_EXP2_BASE_ADDR        FixedPcdGet64 (PcdPciExp2BaseAddr)
#define PCI_EXP2_BASE_SIZE        FixedPcdGet64 (PcdPciExp2BaseSize)
#define PCI_EXP3_BASE_ADDR        FixedPcdGet64 (PcdPciExp3BaseAddr)
#define PCI_EXP3_BASE_SIZE        FixedPcdGet64 (PcdPciExp3BaseSize)
#define DRAM1_BASE_ADDR           FixedPcdGet64 (PcdDram1BaseAddr)
#define DRAM1_SIZE                FixedPcdGet64 (PcdDram1Size)
#define DRAM2_BASE_ADDR           FixedPcdGet64 (PcdDram2BaseAddr)
#define DRAM2_SIZE                FixedPcdGet64 (PcdDram2Size)
#define DRAM3_BASE_ADDR           FixedPcdGet64 (PcdDram3BaseAddr)
#define DRAM3_SIZE                FixedPcdGet64 (PcdDram3Size)
#define QSPI_REGION_BASE_ADDR     FixedPcdGet64 (PcdQspiRegionBaseAddr)
#define QSPI_REGION_SIZE          FixedPcdGet64 (PcdQspiRegionSize)

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param  VirtualMemoryMap  :  Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                               Virtual Memory mapping. This array must be ended by a zero-filled
                               entry

**/

VOID
ArmPlatformGetVirtualMemoryMap (
  IN  ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_ATTRIBUTES  CacheAttributes;
  UINTN                         Index;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  Index = 0;

  ASSERT(VirtualMemoryMap != NULL);

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR*)AllocatePages(
          EFI_SIZE_TO_PAGES (sizeof(ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));

  if (VirtualMemoryTable == NULL) {
    return;
  }

  if (FeaturePcdGet(PcdCacheEnable) == TRUE) {
    CacheAttributes = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;
  } else {
    CacheAttributes = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;
  }

  // DRAM1 (Must be 1st entry)
  VirtualMemoryTable[Index].PhysicalBase = DRAM1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = DRAM1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = DRAM1_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // CCSR Space
  VirtualMemoryTable[++Index].PhysicalBase = CCSR_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CCSR_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CCSR_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // IFC region 1
  //
  // A-009241   : Unaligned write transactions to IFC may result in corruption of data
  // Affects    : IFC
  // Description: 16 byte unaligned write from system bus to IFC may result in extra unintended
  //              writes on external IFC interface that can corrupt data on external flash.
  // Impact     : Data corruption on external flash may happen in case of unaligned writes to
  //              IFC memory space.
  // Workaround: Following are the workarounds:
  //             For write transactions from core, IFC interface memories (including IFC SRAM)
  //                should be configured as device type memory in MMU.
  //             For write transactions from non-core masters (like system DMA), the address
  //                should be 16 byte aligned and the data size should be multiple of 16 bytes.
  //
  VirtualMemoryTable[++Index].PhysicalBase = IFC_REGION1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = IFC_REGION1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = IFC_REGION1_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // QMAN SWP
  VirtualMemoryTable[++Index].PhysicalBase = QMAN_SWP_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = QMAN_SWP_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = QMAN_SWP_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // BMAN SWP
  VirtualMemoryTable[++Index].PhysicalBase = BMAN_SWP_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = BMAN_SWP_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = BMAN_SWP_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // IFC region 2
  VirtualMemoryTable[++Index].PhysicalBase = IFC_REGION2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = IFC_REGION2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = IFC_REGION2_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // DRAM2
  VirtualMemoryTable[++Index].PhysicalBase = DRAM2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = DRAM2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = DRAM2_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // PCIe1
  VirtualMemoryTable[++Index].PhysicalBase = PCI_EXP1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = PCI_EXP1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = PCI_EXP1_BASE_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // PCIe2
  VirtualMemoryTable[++Index].PhysicalBase = PCI_EXP2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = PCI_EXP2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = PCI_EXP2_BASE_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // PCIe3
  VirtualMemoryTable[++Index].PhysicalBase = PCI_EXP3_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = PCI_EXP3_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = PCI_EXP3_BASE_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // DRAM3
  VirtualMemoryTable[++Index].PhysicalBase = DRAM3_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = DRAM3_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = DRAM3_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // QSPI region
  VirtualMemoryTable[++Index].PhysicalBase = QSPI_REGION_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = QSPI_REGION_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = QSPI_REGION_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase = 0;
  VirtualMemoryTable[Index].VirtualBase  = 0;
  VirtualMemoryTable[Index].Length       = 0;
  VirtualMemoryTable[Index].Attributes   = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT((Index + 1) <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}
