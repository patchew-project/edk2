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

#define PCIE_ECAM_BASE       FixedPcdGet64 (PcdPciConfigurationSpaceBaseAddress)
#define PCIE_ECAM_SIZE       FixedPcdGet64 (PcdPciConfigurationSpaceSize)
#define PCIE_IO_BASE         (FixedPcdGet64(PcdPciIoTranslation) + FixedPcdGet64 (PcdPciIoBase))
#define PCIE_IO_SIZE         FixedPcdGet64 (PcdPciIoSize)
#define PCIE_MEM32_BASE      FixedPcdGet64 (PcdPciMmio32Base)
#define PCIE_MEM32_SIZE      FixedPcdGet64 (PcdPciMmio32Size)

#define PCIE_BUS_SIZE        SIZE_1MB

#define PCIE_LINK_TIMEOUT_WAIT_US 1000 // microseconds
#define PCIE_LINK_TIMEOUT_COUNT 1000

#define PCI_TRACE(txt)  DEBUG((DEBUG_VERBOSE, "CDNS_PCI: " txt "\n"))

#define PCIE_ROOTPORT_WRITE32(Add, Val) { UINT32 Value = (UINT32)(Val); CpuIo->Mem.Write (CpuIo,EfiCpuIoWidthUint32,(UINT64)(PcdGet64 (PcdPcieRootPortBaseAddress)+(Add)),1,&Value); }
#define PCIE_ROOTPORT_READ32(Add, Val) { CpuIo->Mem.Read (CpuIo,EfiCpuIoWidthUint32,(UINT64)(PcdGet64 (PcdPcieRootPortBaseAddress)+(Add)),1,&Val); }
#ifdef CDNS_B2B
#define PCIE1_ROOTPORT_WRITE32(Add, Val) { UINT32 Value = (UINT32)(Val); CpuIo->Mem.Write (CpuIo,EfiCpuIoWidthUint32,(UINT64)(PcdGet64 (PcdPcie1RootPortBaseAddress)+(Add)),1,&Value); }
#define PCIE1_ROOTPORT_READ32(Add, Val) { CpuIo->Mem.Read (CpuIo,EfiCpuIoWidthUint32,(UINT64)(PcdGet64 (PcdPcie1RootPortBaseAddress)+(Add)),1,&Val); }
#endif

/*
 * PCIe Core Configuration Register offsets
 */

// Root Port Configuration
#define PCIE_RP                   0x00200000
#define PCIE_PCI_CLASSCODE        0x8

// Local Management
#define PCIE_LM                   0x00100000
#define PCIE_LINK_CTRL_STATUS     0x00
#define PCIE_RP_BAR_CONFIG        0x300

// AXI Configuration
#define PCIE_AXI                   0x00400000

#define PCIE_AXI_REGION_OFF        0x020
#define PCIE_AXI_REGION_TRANS0_OFF 0x000
#define PCIE_AXI_REGION_DESC_OFF   0x008
#define PCIE_AXI_REGION_BASE0_OFF  0x018

#define PCIE_AXI_BAR0_IB         0x800
#define PCIE_AXI_BAR1_IB         0x808
#define PCIE_AXI_NO_BAR_IB       0x810

/*
 * PCIe Core Configuration Register values
 */

#define PCIE_BRIDGE_CLASSCODE    0x06040000
#define PCIE_LINK_UP             0x01
#define PCIE_RCBARPIE            0x19

// AXI Region Address Translation/Base Address bits values
#define PCIE_AXI_BITS_21         20
#define PCIE_AXI_BITS_24         23
#define PCIE_AXI_BITS_25         24
#define PCIE_AXI_BITS_32         31

// AXI Region Outbound PCIe Descriptor Register values
#define PCIE_AXI_DESC_TYPE0      0x80000A
#define PCIE_AXI_DESC_TYPE1      0x80000B
#define PCIE_AXI_DESC_MEM        0x800002
#define PCIE_AXI_DESC_IO         0x800006

#endif
