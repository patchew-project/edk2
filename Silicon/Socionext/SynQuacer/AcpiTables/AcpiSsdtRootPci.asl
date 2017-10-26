/** @file
  Secondary System Description Table (SSDT) for SynQuacer PCIe RCs

  Copyright (c) 2014-2016, ARM Ltd. All rights reserved.<BR>
  Copyright (c) 2017, Linaro Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Platform/Pcie.h>

#include "AcpiTables.h"

DefinitionBlock("SsdtPci.aml", "SSDT", 1, "SNI", "SYNQUACR", EFI_ACPI_OEM_REVISION)
{
  Scope(_SB)
  {
    //
    // PCI Root Complex
    //
//    Device(PCI0)
//    {
//        Name(_HID, EISAID("PNP0A08"))   // PCI Express Root Bridge
//        Name(_CID, EISAID("PNP0A03"))   // Compatible PCI Root Bridge
//        Name(_SEG, Zero)                // PCI Segment Group number
//        Name(_BBN, Zero)                // PCI Base Bus Number
//        Name(_CCA, 1)                   // Cache Coherency Attribute
//
//        // PCI Routing Table
//        Name(_PRT, Package() {
//            Package () { 0xFFFF, 0, Zero, 222 },   // INTA
//            Package () { 0xFFFF, 1, Zero, 222 },   // INTB
//            Package () { 0xFFFF, 2, Zero, 222 },   // INTC
//            Package () { 0xFFFF, 3, Zero, 222 },   // INTD
//        })
//        // Root complex resources
//        Method (_CRS, 0, Serialized) {
//            Name (RBUF, ResourceTemplate () {
//                WordBusNumber ( // Bus numbers assigned to this root
//                    ResourceProducer,
//                    MinFixed, MaxFixed, PosDecode,
//                    0,                              // AddressGranularity
//                    SYNQUACER_PCI_SEG0_BUSNUM_MIN,  // AddressMinimum - Minimum Bus Number
//                    SYNQUACER_PCI_SEG0_BUSNUM_MIN,  // AddressMaximum - Maximum Bus Number
//                    0,                              // AddressTranslation - Set to 0
//                    1                               // RangeLength - Number of Busses
//                )
//
//                DWordMemory ( // 32-bit BAR Windows
//                    ResourceProducer, PosDecode,
//                    MinFixed, MaxFixed,
//                    Cacheable, ReadWrite,
//                    0x00000000,                         // Granularity
//                    SYNQUACER_PCI_SEG0_MMIO32_MIN,      // Min Base Address
//                    SYNQUACER_PCI_SEG0_MMIO32_MAX,      // Max Base Address
//                    0x00000000,                         // Translate
//                    SYNQUACER_PCI_SEG0_MMIO32_SIZE      // Length
//                )
//
//                QWordMemory ( // 64-bit BAR Windows
//                    ResourceProducer, PosDecode,
//                    MinFixed, MaxFixed,
//                    Cacheable, ReadWrite,
//                    0x00000000,                         // Granularity
//                    SYNQUACER_PCI_SEG0_MMIO64_MIN,      // Min Base Address
//                    SYNQUACER_PCI_SEG0_MMIO64_MAX,      // Max Base Address
//                    0x00000000,                         // Translate
//                    SYNQUACER_PCI_SEG0_MMIO64_SIZE      // Length
//                )
//
//                DWordIo ( // IO window
//                    ResourceProducer,
//                    MinFixed,
//                    MaxFixed,
//                    PosDecode,
//                    EntireRange,
//                    0x00000000,                         // Granularity
//                    SYNQUACER_PCI_SEG0_PORTIO_MIN,      // Min Base Address
//                    SYNQUACER_PCI_SEG0_PORTIO_MAX,      // Max Base Address
//                    SYNQUACER_PCI_SEG0_PORTIO_MEMBASE,  // Translate
//                    SYNQUACER_PCI_SEG0_PORTIO_SIZE,     // Length
//                    ,
//                    ,
//                    ,
//                    TypeTranslation
//                )
//            }) // Name(RBUF)
//
//            Return (RBUF)
//        } // Method(_CRS)
//
//        Device (RES0)
//        {
//            Name (_HID, "PNP0C02")
//            Name (_CRS, ResourceTemplate ()
//            {
//                Memory32Fixed (ReadWrite,
//                               SYNQUACER_PCI_SEG0_CONFIG_BASE,
//                               SYNQUACER_PCI_SEG0_CONFIG_SIZE)
//            })
//        }
//
//        //
//        // OS Control Handoff
//        //
//        Name(SUPP, Zero) // PCI _OSC Support Field value
//        Name(CTRL, Zero) // PCI _OSC Control Field value
//
//        /*
//      See [1] 6.2.10, [2] 4.5
//        */
//        Method(_OSC,4) {
//            // Check for proper UUID
//            If(LEqual(Arg0,ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {
//                // Create DWord-adressable fields from the Capabilities Buffer
//                CreateDWordField(Arg3,0,CDW1)
//                CreateDWordField(Arg3,4,CDW2)
//                CreateDWordField(Arg3,8,CDW3)
//
//                // Save Capabilities DWord2 & 3
//                Store(CDW2,SUPP)
//                Store(CDW3,CTRL)
//
//                // Only allow native hot plug control if OS supports:
//                // * ASPM
//                // * Clock PM
//                // * MSI/MSI-X
//                If(LNotEqual(And(SUPP, 0x16), 0x16)) {
//                    And(CTRL,0x1E,CTRL) // Mask bit 0 (and undefined bits)
//                }
//
//                // Always allow native PME, AER (no dependencies)
//
//                // Never allow SHPC (no SHPC controller in this system)
//                And(CTRL,0x1D,CTRL)
//
//                If(LNotEqual(Arg1,One)) {    // Unknown revision
//                    Or(CDW1,0x08,CDW1)
//                }
//
//                If(LNotEqual(CDW3,CTRL)) {    // Capabilities bits were masked
//                    Or(CDW1,0x10,CDW1)
//                }
//                // Update DWORD3 in the buffer
//                Store(CTRL,CDW3)
//                Return(Arg3)
//            } Else {
//                Or(CDW1,4,CDW1) // Unrecognized UUID
//                Return(Arg3)
//            }
//        } // End _OSC
//    } // PCI0

    Device(PCI1)
    {
        Name(_HID, EISAID("PNP0A08"))   // PCI Express Root Bridge
        Name(_CID, EISAID("PNP0A03"))   // Compatible PCI Root Bridge
        Name(_SEG, 1)                   // PCI Segment Group number
        Name(_BBN, Zero)                // PCI Base Bus Number
        Name(_CCA, 1)                   // Cache Coherency Attribute

        // PCI Routing Table
        Name(_PRT, Package() {
            Package () { 0xFFFF, 0, Zero, 214 },   // INTA
            Package () { 0xFFFF, 1, Zero, 214 },   // INTB
            Package () { 0xFFFF, 2, Zero, 214 },   // INTC
            Package () { 0xFFFF, 3, Zero, 214 },   // INTD
        })
        // Root complex resources
        Method (_CRS, 0, Serialized) {
            Name (RBUF, ResourceTemplate () {
                WordBusNumber ( // Bus numbers assigned to this root
                    ResourceProducer,
                    MinFixed, MaxFixed, PosDecode,
                    0,                              // AddressGranularity
                    SYNQUACER_PCI_SEG1_BUSNUM_MIN,  // AddressMinimum - Minimum Bus Number
                    SYNQUACER_PCI_SEG1_BUSNUM_MIN,  // AddressMaximum - Maximum Bus Number
                    0,                              // AddressTranslation - Set to 0
                    1                               // RangeLength - Number of Busses
                )

                DWordMemory ( // 32-bit BAR Windows
                    ResourceProducer, PosDecode,
                    MinFixed, MaxFixed,
                    Cacheable, ReadWrite,
                    0x00000000,                         // Granularity
                    SYNQUACER_PCI_SEG1_MMIO32_MIN,      // Min Base Address
                    SYNQUACER_PCI_SEG1_MMIO32_MAX,      // Max Base Address
                    0x00000000,                         // Translate
                    SYNQUACER_PCI_SEG1_MMIO32_SIZE      // Length
                )

                QWordMemory ( // 64-bit BAR Windows
                    ResourceProducer, PosDecode,
                    MinFixed, MaxFixed,
                    Cacheable, ReadWrite,
                    0x00000000,                         // Granularity
                    SYNQUACER_PCI_SEG1_MMIO64_MIN,      // Min Base Address
                    SYNQUACER_PCI_SEG1_MMIO64_MAX,      // Max Base Address
                    0x00000000,                         // Translate
                    SYNQUACER_PCI_SEG1_MMIO64_SIZE      // Length
                )

                DWordIo ( // IO window
                    ResourceProducer,
                    MinFixed,
                    MaxFixed,
                    PosDecode,
                    EntireRange,
                    0x00000000,                         // Granularity
                    SYNQUACER_PCI_SEG1_PORTIO_MIN,      // Min Base Address
                    SYNQUACER_PCI_SEG1_PORTIO_MAX,      // Max Base Address
                    SYNQUACER_PCI_SEG1_PORTIO_MEMBASE,  // Translate
                    SYNQUACER_PCI_SEG1_PORTIO_SIZE,     // Length
                    ,
                    ,
                    ,
                    TypeTranslation
                )
            }) // Name(RBUF)

            Return (RBUF)
        } // Method(_CRS)

        Device (RES0)
        {
            Name (_HID, "PNP0C02")
            Name (_CRS, ResourceTemplate ()
            {
                Memory32Fixed (ReadWrite,
                               SYNQUACER_PCI_SEG1_CONFIG_BASE,
                               SYNQUACER_PCI_SEG1_CONFIG_SIZE)
            })
        }

        //
        // OS Control Handoff
        //
        Name(SUPP, Zero) // PCI _OSC Support Field value
        Name(CTRL, Zero) // PCI _OSC Control Field value

        /*
      See [1] 6.2.10, [2] 4.5
        */
        Method(_OSC,4) {
            // Check for proper UUID
            If(LEqual(Arg0,ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {
                // Create DWord-adressable fields from the Capabilities Buffer
                CreateDWordField(Arg3,0,CDW1)
                CreateDWordField(Arg3,4,CDW2)
                CreateDWordField(Arg3,8,CDW3)

                // Save Capabilities DWord2 & 3
                Store(CDW2,SUPP)
                Store(CDW3,CTRL)

                // Only allow native hot plug control if OS supports:
                // * ASPM
                // * Clock PM
                // * MSI/MSI-X
                If(LNotEqual(And(SUPP, 0x16), 0x16)) {
                    And(CTRL,0x1E,CTRL) // Mask bit 0 (and undefined bits)
                }

                // Always allow native PME, AER (no dependencies)

                // Never allow SHPC (no SHPC controller in this system)
                And(CTRL,0x1D,CTRL)

                If(LNotEqual(Arg1,One)) {    // Unknown revision
                    Or(CDW1,0x08,CDW1)
                }

                If(LNotEqual(CDW3,CTRL)) {    // Capabilities bits were masked
                    Or(CDW1,0x10,CDW1)
                }
                // Update DWORD3 in the buffer
                Store(CTRL,CDW3)
                Return(Arg3)
            } Else {
                Or(CDW1,4,CDW1) // Unrecognized UUID
                Return(Arg3)
            }
        } // End _OSC
    } // PCI0
  }
}
