/** @file
*  Header for Cadence PCIe Root Complex
*
*  Copyright (c) 2011-2015, ARM Ltd. All rights reserved.
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

#ifndef __CDNS_PCI_H__
#define __CDNS_PCI_H__

#include <Protocol/CpuIo2.h>

// Define this if using Back to Back config, EP will be configured with NIC ID
#define CDNS_B2B

#define PCI_ECAM_BASE       FixedPcdGet64 (PcdPciConfigurationSpaceBaseAddress)
#define PCI_ECAM_SIZE       FixedPcdGet64 (PcdPciConfigurationSpaceSize)
#define PCI_IO_BASE         FixedPcdGet64 (PcdPciIoBase)
#define PCI_IO_SIZE         FixedPcdGet64 (PcdPciIoSize)
#define PCI_MEM32_BASE      FixedPcdGet64 (PcdPciMmio32Base)
#define PCI_MEM32_SIZE      FixedPcdGet64 (PcdPciMmio32Size)
#define PCI_MEM64_BASE      FixedPcdGet64 (PcdPciMmio64Base)
#define PCI_MEM64_SIZE      FixedPcdGet64 (PcdPciMmio64Size)

#define PCI_BUS_SIZE        0x00100000

#define PCI_TRACE(txt)  DEBUG((EFI_D_VERBOSE, "CDNS_PCI: " txt "\n"))

#define PCIE_ROOTPORT_WRITE32(Add, Val) { UINT32 Value = (UINT32)(Val); CpuIo->Mem.Write (CpuIo,EfiCpuIoWidthUint32,(UINT64)(PcdGet64 (PcdPcieRootPortBaseAddress)+(Add)),1,&Value); }
#define PCIE_ROOTPORT_READ32(Add, Val) { CpuIo->Mem.Read (CpuIo,EfiCpuIoWidthUint32,(UINT64)(PcdGet64 (PcdPcieRootPortBaseAddress)+(Add)),1,&Val); }
#ifdef CDNS_B2B
#define PCIE1_ROOTPORT_WRITE32(Add, Val) { UINT32 Value = (UINT32)(Val); CpuIo->Mem.Write (CpuIo,EfiCpuIoWidthUint32,(UINT64)(PcdGet64 (PcdPcie1RootPortBaseAddress)+(Add)),1,&Value); }
#define PCIE1_ROOTPORT_READ32(Add, Val) { CpuIo->Mem.Read (CpuIo,EfiCpuIoWidthUint32,(UINT64)(PcdGet64 (PcdPcie1RootPortBaseAddress)+(Add)),1,&Val); }
#endif

/*
 * Bridge Internal Registers
 */

#define PCIE_RP                  0x00200000
#define PCIE_LM                  0x00100000
#define PCIE_AXI                 0x00400000
#define PCIE_PCI_CLASSCODE       0x8
#define PCIE_RP_BAR_CONFIG       0x300
#define RCBARPIE                 0x19
#define PCIE_BAR0_IB             0x800
#define PCIE_BAR1_IB             0x808
#define PCIE_NO_BAR_IB           0x810

#define PCIE_BRIDGE_CLASSCODE    0x06040000

#define AXI_PCIE_TYPE0           0x80000A
#define AXI_PCIE_TYPE1           0x80000B
#define AXI_PCIE_MEM             0x800002
#define AXI_PCIE_IO              0x800006

#define LINK_CTRL_STATUS         0x00
#define LINK_UP                  0x01

#endif
