/** @file
*  Initialize the Cadence PCIe Root complex
*
*  Copyright (c) 2017, Cadence Design Systems. All rights reserved.
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

#include <Library/BaseLib.h>
#include <Library/CspSysReg.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/Cpu.h>

#include "CdnsPci.h"

STATIC
VOID
CdnsPciConfigRegion (
  EFI_CPU_IO2_PROTOCOL *CpuIo,
  IN UINT32 Region,
  IN UINT32 Descriptor,
  IN UINT32 TransAddr,
  IN UINT32 TransAddrBits,
  IN UINT32 BaseAddr,
  IN UINT32 BaseAddrBits
  )
{
  UINTN RegionBaseAddr = PCIE_AXI + (Region * PCIE_AXI_REGION_OFF);

  PCIE_ROOTPORT_WRITE32 (RegionBaseAddr + PCIE_AXI_REGION_TRANS0_OFF,
                         TransAddr | TransAddrBits);
  PCIE_ROOTPORT_WRITE32 (RegionBaseAddr + PCIE_AXI_REGION_DESC_OFF,
                         Descriptor);
  PCIE_ROOTPORT_WRITE32 (RegionBaseAddr + PCIE_AXI_REGION_BASE0_OFF,
                         BaseAddr | BaseAddrBits);
}

STATIC
VOID
CdnsPciRegInit(
  EFI_CPU_IO2_PROTOCOL    *CpuIo
)
{
  UINT32                  Value;

  // Setup the class code as PCIe Host Bridge.
  PCIE_ROOTPORT_WRITE32 (PCIE_RP + PCIE_PCI_CLASSCODE, PCIE_BRIDGE_CLASSCODE);

  // Set up the BARs via the Root Port registers
  PCIE_ROOTPORT_READ32 (PCIE_LM + PCIE_RP_BAR_CONFIG, Value);
  PCIE_ROOTPORT_WRITE32 (PCIE_LM + PCIE_RP_BAR_CONFIG, Value | (1 << PCIE_RCBARPIE));

  // Allow incoming writes
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_BAR0_IB, PCIE_AXI_BITS_32);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_BAR1_IB, PCIE_AXI_BITS_32);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_NO_BAR_IB, PCIE_AXI_BITS_32);

  // Set up region 0 for Type 0 write (bus 0 and 1), size 2MB
  CdnsPciConfigRegion (
          CpuIo,
          0,
          PCIE_AXI_DESC_TYPE0,
          PCIE_ECAM_BASE,
          PCIE_AXI_BITS_25,
          0,
          PCIE_AXI_BITS_21
          );

  // Set up region 1 for Type 1 writes (bus 2 upwards), size (32-2)MB
  CdnsPciConfigRegion(
          CpuIo,
          1,
          PCIE_AXI_DESC_TYPE1,
          PCIE_ECAM_BASE + (2*PCIE_BUS_SIZE),
          PCIE_AXI_BITS_25,
          2*PCIE_BUS_SIZE,
          PCIE_AXI_BITS_25
          );

  // Set up region 2 for memory write, size 16MB
  CdnsPciConfigRegion(
          CpuIo,
          2,
          PCIE_AXI_DESC_MEM,
          PCIE_MEM32_BASE,
          PCIE_AXI_BITS_25,
          (PCIE_MEM32_BASE - PCIE_ECAM_BASE),
          PCIE_AXI_BITS_24
          );

  // Set up region 3 for IO write, size 16MB
  CdnsPciConfigRegion(
          CpuIo,
          3,
          PCIE_AXI_DESC_IO,
          PCIE_IO_BASE,
          PCIE_AXI_BITS_25,
          (PCIE_IO_BASE - PCIE_ECAM_BASE),
          PCIE_AXI_BITS_24
          );
}

EFI_STATUS
HWPciRbInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32                  Count;
  EFI_CPU_IO2_PROTOCOL    *CpuIo;
  EFI_STATUS              Status;
  UINT32                  Value;

  PCI_TRACE ("HWPciRbInit()");

  PCI_TRACE ("PCIe Setting up Address Translation");

  Status = gBS->LocateProtocol (&gEfiCpuIo2ProtocolGuid, NULL,
                  (VOID **)&CpuIo);
  ASSERT_EFI_ERROR (Status);

  // Check for link up
  for (Count = 0; Count < PCIE_LINK_TIMEOUT_COUNT; Count++) {
    gBS->Stall (PCIE_LINK_TIMEOUT_WAIT_US);
    PCIE_ROOTPORT_READ32 (PCIE_LM + PCIE_LINK_CTRL_STATUS, Value);
    if (Value & PCIE_LINK_UP) {
      break;
    }
  }
  if (!(Value & PCIE_LINK_UP)) {
    DEBUG ((DEBUG_ERROR, "PCIe link not up: %x.\n", Value));
    return EFI_NOT_READY;
  }

  // Initialise configuration registers
  CdnsPciRegInit(CpuIo);

  return EFI_SUCCESS;
}
