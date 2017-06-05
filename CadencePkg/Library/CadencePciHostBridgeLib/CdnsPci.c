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
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_BAR0_IB, 0x1f);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_BAR1_IB, 0x1f);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_NO_BAR_IB, 0x1f);

  // Set up an area for Type 0 write
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG0_ADDR0, 0x18);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG0_DESC0, PCIE_AXI_TYPE0);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG0_AXI_ADDR0, 0x14);

  // Set up an area for Type 1 write
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG1_ADDR0, PCI_ECAM_BASE + (2*PCI_BUS_SIZE) + 0x18);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG1_DESC0, PCIE_AXI_TYPE1);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG1_AXI_ADDR0, (2*PCI_BUS_SIZE) + 0x18);

  // Set up an area for memory write
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG2_ADDR0, PCI_MEM32_BASE + 0x18);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG2_DESC0, PCIE_AXI_MEM);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG2_AXI_ADDR0, PCI_ECAM_SIZE + 0x17);

  // Set up an area for IO write
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG3_ADDR0, PCI_IO_BASE + 0x18);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG3_DESC0, PCIE_AXI_IO);
  PCIE_ROOTPORT_WRITE32 (PCIE_AXI + PCIE_AXI_REG3_AXI_ADDR0, (PCI_ECAM_SIZE + PCI_MEM32_SIZE) + 0x17 );
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
  for (Count = 0; Count < PCI_LINK_TIMEOUT_COUNT; Count++) {
    gBS->Stall (PCI_LINK_TIMEOUT_WAIT_US);
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
