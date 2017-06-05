/*
* Copyright (c) 2013, Al Stone <al.stone@linaro.org>
* Copyright (c) 2017, Cadence Design Systems, Inc. All rights reserved.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
* NB: This License is also known as the "BSD 2-Clause License".
*
*
*
*/

DefinitionBlock (
  "dsdt.aml",    // output filename
  "DSDT",      // table signature
  2,      // DSDT compliance revision
  "CDNS",    // OEM ID
  "CDNSCSP0",    // table ID
  0x00000001)    // OEM revision
{
  Scope (\_SB)
  {
    Method (_OSC, 4, NotSerialized)
    {
      /* Platform-Wide OSPM Capabilities */
      If(LEqual(Arg0,ToUUID("0811B06E-4A27-44F9-8D60-3CBBC22E7B48")))
      {
        /* APEI support unconditionally */
        Return (Arg3)
      } Else {
        CreateDWordField (Arg3, Zero, CDW1)
        /* Set invalid UUID error bit */
        Or (CDW1, 0x04, CDW1)
        Return (Arg3)
      }
    }

    //
    // One Emulated aarch64 CPU  with 1 core
    //
    Device(CPU0) { // Cluster 0, Cpu 0
      Name(_HID, "ACPI0007")
      Name(_UID, 0)
    }
    // Cadence UART
    Device(COM0) {
      Name(_HID, "CDNS0001")
      Name(_UID, 0)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0xFD000000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x20 }
      })
    }

    //Legacy IRQs
    Name (PR04, Package (0x04)
    {
      Package (0x04)
      {
        0xFFFF, Zero, LNKA, Zero
      },
      Package (0x04)
      {
        0xFFFF, Zero, LNKB, Zero
      },
      Package (0x04)
      {
        0xFFFF, Zero, LNKC, Zero
      },
      Package (0x04)
      {
        0xFFFF, Zero, LNKD, Zero
      }
    })
    Device (LNKA)
    {
      Name (_HID, Eisaid ("PNP0C0F"))
      Name (_UID, 1)
      Name(_PRS, ResourceTemplate(){
        Interrupt(ResourceProducer, Level, ActiveHigh, Exclusive, , ,) { 46 }
      })
      Method(_DIS) {}
      Method(_CRS) { Return (_PRS) }
      Method(_SRS, 1) {}
    }
    Device (LNKB)
    {
      Name (_HID, Eisaid ("PNP0C0F"))
      Name (_UID, 1)
      Name(_PRS, ResourceTemplate(){
        Interrupt(ResourceProducer, Level, ActiveHigh, Exclusive, , ,) { 46 }
      })
      Method(_DIS) {}
      Method(_CRS) { Return (_PRS) }
      Method(_SRS, 1) {}
    }
    Device (LNKC)
    {
      Name (_HID, Eisaid ("PNP0C0F"))
      Name (_UID, 1)
      Name(_PRS, ResourceTemplate(){
        Interrupt(ResourceProducer, Level, ActiveHigh, Exclusive, , ,) { 46 }
      })
      Method(_DIS) {}
      Method(_CRS) { Return (_PRS) }
      Method(_SRS, 1) {}
    }
    Device (LNKD)
    {
      Name (_HID, Eisaid ("PNP0C0F"))
      Name (_UID, 1)
      Name(_PRS, ResourceTemplate(){
        Interrupt(ResourceProducer, Level, ActiveHigh, Exclusive, , ,) { 46 }
      })
      Method(_DIS) {}
      Method(_CRS) { Return (_PRS) }
      Method(_SRS, 1) {}
    }

    Device (PCI0)
    {
      Name (_HID, EisaId ("PNP0A08"))  // _HID: Hardware ID
      Name (_CID, EisaId ("PNP0A03"))  // _CID: Compatible ID
      Name (_SEG, 0x00)  // _SEG: PCI Segment
      Name (_BBN, 0x00)  // _BBN: BIOS Bus Number
      Name(_ADR,Zero)
      NAME(_CCA,0)  // Cache Coherent Architecture = FALSE
      //  Name(_CBA,0x50000000)

      //
      // OS Control Handoff
      //
      Name(SUPP, Zero) // PCI _OSC Support Field value
      Name(CTRL, Zero) // PCI _OSC Control Field value
      Method (_OSC, 4, Serialized)  // _OSC: Operating System Capabilities
      {
        Store (Arg3, Local0)
        CreateDWordField (Local0, Zero, CDW1)
        CreateDWordField (Local0, 0x04, CDW2)
        CreateDWordField (Local0, 0x08, CDW3)
        // Save Capabilities DWord2 & 3
        Store(CDW2,SUPP)
        Store(CDW3,CTRL)
        // Never allow SHPC (no SHPC controller in this system)
        And(CTRL,0x1D,CTRL)
        If(LNotEqual(Arg1,One)) { // Unknown revision
          Or(CDW1,0x08,CDW1)
        }
        // Update DWORD3 in the buffer
        Store(CTRL,CDW3)
        Return (Local0)
      }


      Name (_UID, Zero)  // _UID: Unique ID
      Device (RP01)
      {
        Name (_ADR, 0x001C0000)  // _ADR: Address
        OperationRegion (PXCS, PCI_Config, Zero, 0x0380)
        Field (PXCS, AnyAcc, NoLock, Preserve)
        {
          VDID,   32,
          Offset (0x19),
          SCBN,   8,
          Offset (0x50),
          L0SE,   1,
           ,   3,
          LDIS,   1,
          Offset (0x51),
          Offset (0x52),
           ,   13,
          LASX,   1,
          Offset (0x54),
           ,   6,
          HPCE,   1,
          Offset (0x5A),
          ABPX,   1,
           ,   2,
          PDCX,   1,
           ,   2,
          PDSX,   1,
          Offset (0x5B),
          Offset (0x60),
          Offset (0x62),
          PSPX,   1,
          PMEP,   1,
          Offset (0xA4),
          D3HT,   2,
          Offset (0xD8),
           ,   30,
          HPEX,   1,
          PMEX,   1,
          Offset (0xE2),
           ,   2,
          L23E,   1,
          L23R,   1,
          Offset (0x324),
           ,   3,
          LEDM,   1
        }

        Field (PXCS, AnyAcc, NoLock, WriteAsZeros)
        {
          Offset (0xDC),
           ,   30,
          HPSX,   1,
          PMSX,   1
        }

        Method (_STA, 0, NotSerialized)  // _STA: Status
        {
          Return (0x0F)
        }

        Name (LTRV, Package (0x04)
        {
          Zero,
          Zero,
          Zero,
          Zero
        })

        Name (OPTS, Zero)

        Name (RPAV, Zero)

        Method (_REG, 2, NotSerialized)  // _REG: Region Availability
        {
          If (LAnd (LEqual (Arg0, 0x02), LEqual (Arg1, One)))
          {
            Store (One, RPAV)
          }
        }

        Method (HPME, 0, Serialized)
        {
          If (LOr (PSPX, PMEP))
          {
            Store (PMEX, Local1)
            Store (Zero, PMEX)
            Sleep (0x32)
            Store (One, PSPX)
            Sleep (0x32)
            If (PSPX)
            {
              Store (One, PSPX)
              Sleep (0x32)
            }
            Store (Local1, PMEX)
          }
          If (PMSX)
          {
            Store (0xC8, Local0)
            While (Local0)
            {
              Store (One, PMSX)
              If (PMSX)
              {
                Decrement (Local0)
              }
              Else
              {
                Store (Zero, Local0)
              }
            }
          }
        }
      }
      Name(_PRT, Package()
      {
        Package(){0x0000ffff, 0, LNKA, 0},  // Slot 1, INTA
        Package(){0x0000ffff, 1, LNKB, 0},  // Slot 1, INTB
        Package(){0x0000ffff, 2, LNKC, 0},  // Slot 1, INTC
        Package(){0x0000ffff, 3, LNKD, 0},  // Slot 1, INTD
      })
      Method (_CRS, 0, Serialized)
      {
        Name (PRT0, ResourceTemplate ()
        {
          /* bus number is from 0 - 1f */
          WordBusNumber (ResourceConsumer, MinFixed, MaxFixed, SubDecode,
                        0x0000,
                        0x0000,
                        0x001f,
                        0x0000,
                        0x0020)
          DWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                        0x00000000,
                        0x42000000,
                        0x42FFFFFF,
                        0x00000000,
                        0x01000000)
          DWordIO (ResourceConsumer, MinFixed, MaxFixed, PosDecode, EntireRange,
                        0x00000000,
                        0x43000000,
                        0x43FFFFFF,
                        0x00000000,
                        0x01000000)
        })
      Return (PRT0)
    }
  }

  //  Device(COM0) {
  //    Name(_HID, "ARMH0011")
  //    Name(_CID, "PL011")
  //    Name(_UID, Zero)

  //    Name(_CRS, ResourceTemplate() {
  //      Memory32Fixed(ReadWrite, 0x1c090000, 0x1000)
  //      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x25 }
  //    })
  //  }
  }
}
