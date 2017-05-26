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

#include <Library/CspSysReg.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/Cpu.h>

#include "CdnsPci.h"

EFI_STATUS
HWPciRbInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32                  Value;
  UINT32                  Index;
  EFI_CPU_IO2_PROTOCOL    *CpuIo;
  EFI_STATUS              Status;

  PCI_TRACE ("HWPciRbInit()");

  PCI_TRACE ("PCIe Setting up Address Translation");

  Status = gBS->LocateProtocol (&gEfiCpuIo2ProtocolGuid, NULL,
                  (VOID **)&CpuIo);
  ASSERT_EFI_ERROR (Status);

  // Check for link up
  for (Index = 0; Index < 1000; Index++) {
    gBS->Stall (1000);
    PCIE_ROOTPORT_READ32 (PCIE_LM + LINK_CTRL_STATUS, Value);
    if (Value & LINK_UP) {
      break;
    }
  }
  if (!(Value & LINK_UP)) {
    DEBUG ((EFI_D_ERROR, "PCIe link not up: %x.\n", Value));
    return EFI_NOT_READY;
  }

  // Setup the class code as PCIe Host Bridge.
  PCIE_ROOTPORT_WRITE32 (PCIE_RP + PCIE_PCI_CLASSCODE, PCIE_BRIDGE_CLASSCODE);

  // Set up the BARs via the Root Port registers
  PCIE_ROOTPORT_READ32 (PCIE_LM + PCIE_RP_BAR_CONFIG, Value);
  PCIE_ROOTPORT_WRITE32(PCIE_LM + PCIE_RP_BAR_CONFIG, Value| (1 << RCBARPIE));

  // Allow incoming writes
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + PCIE_BAR0_IB,0x1f );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + PCIE_BAR1_IB,0x1f );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + PCIE_NO_BAR_IB,0x1f );

  // Set up an area for Type 0 write
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x0,0x18 );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x8,AXI_PCIE_TYPE0 );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x18,0x14 );

  // Set up an area for Type 1 write
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x20,PCI_ECAM_BASE + (2*PCI_BUS_SIZE) + 0x18 );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x28,AXI_PCIE_TYPE1 );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x38,(2*PCI_BUS_SIZE) + 0x18 );

  // Set up an area for memory write
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x40,PCI_MEM32_BASE + 0x18 );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x48,AXI_PCIE_MEM );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x58,PCI_ECAM_SIZE + 0x17 );

  // Set up an area for IO write
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x60,PCI_IO_BASE + 0x18 );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x68,AXI_PCIE_IO );
  PCIE_ROOTPORT_WRITE32(PCIE_AXI + 0x78,(PCI_ECAM_SIZE + PCI_MEM32_SIZE) + 0x17 );

#ifdef CDNS_B2B
  // Set up EP device as Intel Ethernet
  //PCIE1_ROOTPORT_WRITE32(0x200000,0x15338086 );
  //PCIE1_ROOTPORT_WRITE32(0x100044,0x00008086 );
  // Set up EP device as ASM1062 SATA
  PCIE1_ROOTPORT_WRITE32(0x200000,0x06121b21 );
  PCIE1_ROOTPORT_WRITE32(0x100044,0x00001b21 );
  PCIE1_ROOTPORT_WRITE32 (PCIE_RP + PCIE_PCI_CLASSCODE, 0x01060101);
  // Set up EP BAR as 4M
  PCIE1_ROOTPORT_WRITE32(0x100240,0xaf );
#endif

  return EFI_SUCCESS;
}
