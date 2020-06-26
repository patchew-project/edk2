/** @file
*  Functions in this library instance make use of MMIO functions in IoLib to
*  access memory mapped PCI configuration space.
*
*  All assertions for I/O operations are handled in MMIO functions in the IoLib
*  Library.
*
*  Copyright (c) 2006 - 2019, Intel Corporation. All rights reserved.
*
*  slave error is generated when host accesses the configuration space of
*  non-available device or unimplemented function on a given bus. So this
*  library introduces a workaround using CheckBdfValidity(), to return
*  0xffffffff for all such access.
*
*  Copyright (c) 2020, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/PciExpressLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <N1SdpPlatform.h>

/**
  Assert the validity of a PCI address. A valid PCI address should contain 1's
  only in the low 32 bits.

  @param  A The address to validate.
**/
#define ASSERT_INVALID_PCI_ADDRESS(A) \
  ASSERT (((A) & ~0xffffffff) == 0)

#define EFI_PCIE_ADDRESS(bus, dev, func, reg) \
  (UINT64) ( \
  (((UINTN) bus) << 20) | \
  (((UINTN) dev) << 15) | \
  (((UINTN) func) << 12) | \
  (((UINTN) (reg)) < 4096 ? ((UINTN) (reg)) : (UINT64) (LShiftU64 ((UINT64) (reg), 32))))

#define GET_PCIE_BASE_ADDRESS(Address)  (Address & 0xF8000000)

/* Root port Entry, BDF Entries Count */
#define BDF_TABLE_ENTRY_SIZE    4
#define BDF_TABLE_HEADER_COUNT  2
#define BDF_TABLE_HEADER_SIZE   8

/* BDF table offsets for PCIe */
#define PCIE_BDF_TABLE_OFFSET   0

#define GET_BUS_NUM(Address)    ((Address>>20) & 0x7f)
#define GET_DEV_NUM(Address)    ((Address>>15) & 0x1f)
#define GET_FUNC_NUM(Address)   ((Address>>12) & 0x07)
#define GET_REG_NUM(Address)    ((Address) & 0xFFF)

/**
  BDF Table structure : (Header + BDF Entries)
  -------------------
  ROOT PORT ADDRESS
  BDF ENTRIES COUNT
  BDF ENTRY 0
  BDF ENTRY 1
  BDF ENTRY 2
  BDF ENTRY 3
  BDF ENTRY 4
  ...
  BDF ENTRY N
  ------------------
**/

UINTN DummyPciData = 0xffffffff;

/**
  Registers a PCI device so PCI configuration registers may be accessed after
  SetVirtualAddressMap().

  Registers the PCI device specified by Address so all the PCI configuration
  registers associated with that PCI device may be accessed after SetVirtualAddressMap()
  is called.

  If Address > 0x0FFFFFFF, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @retval RETURN_SUCCESS           The PCI device was registered for runtime access.
  @retval RETURN_UNSUPPORTED       An attempt was made to call this function
                                   after ExitBootServices().
  @retval RETURN_UNSUPPORTED       The resources required to access the PCI device
                                   at runtime could not be mapped.
  @retval RETURN_OUT_OF_RESOURCES  There are not enough resources available to
                                   complete the registration.
**/
RETURN_STATUS
EFIAPI
PciExpressRegisterForRuntimeAccess (
  IN      UINTN                     Address
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return RETURN_UNSUPPORTED;
}

/**
  Checks if the incoming PCI address is a valid BDF address.

  SCP performs the initial bus scan and prepares a table of valid BDF addresses
  and shares them through non-trusted SRAM. This function validates if the PCI
  address from any PCI request falls within the table of valid entries. If not,
  this function will return false. This is a workaround to avoid bus fault that
  happens when accessing unavailable PCI device due to RTL bug.

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @return The base address of PCI Express.
**/
STATIC
BOOLEAN
CheckBdfValidity (
  IN      UINTN                     Address
  )
{
  UINT8 Bus, Device, Function;
  UINTN BdfCount, BdfValue;
  UINTN BdfEntry;
  UINTN Count;
  UINTN TableBase;
  UINTN PciAddress;

  Bus = GET_BUS_NUM (Address);
  Device = GET_DEV_NUM (Address);
  Function = GET_FUNC_NUM (Address);

  PciAddress = EFI_PCIE_ADDRESS (Bus, Device, Function, 0);

  if (GET_PCIE_BASE_ADDRESS (Address) == FixedPcdGet64 (PcdPciExpressBaseAddress)) {
    TableBase = N1SDP_NON_SECURE_SRAM_BASE + PCIE_BDF_TABLE_OFFSET;
  }

  BdfCount = MmioRead32 (TableBase + BDF_TABLE_ENTRY_SIZE);
  BdfEntry = TableBase + BDF_TABLE_HEADER_SIZE;

  /* Skip the header & check remaining entry */
  for (Count = 0; Count < BdfCount; Count++, BdfEntry += BDF_TABLE_ENTRY_SIZE) {
    BdfValue = MmioRead32 (BdfEntry);
    if (BdfValue == PciAddress) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Gets the base address of PCI Express.

  This internal functions retrieves PCI Express Base Address via a PCD entry.

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @return The base address of PCI Express.
**/
STATIC
VOID*
GetPciExpressAddress (
  IN      UINTN                     Address
  )
{
  UINT8 Bus, Device, Function;
  UINT16 Register;
  UINTN ConvAddress;

  // Get the EFI notation
  Bus = GET_BUS_NUM (Address);
  Device = GET_DEV_NUM (Address);
  Function = GET_FUNC_NUM (Address);
  Register = GET_REG_NUM (Address);

  if ((Bus == 0) && (Device == 0) && (Function == 0)) {
    ConvAddress = PcdGet32 (PcdPcieRootPortConfigBaseAddress +
                         EFI_PCIE_ADDRESS (Bus, Device, Function, Register));
  } else {
    ConvAddress = PcdGet64 (PcdPciExpressBaseAddress +
                         EFI_PCIE_ADDRESS (Bus, Device, Function, Register));
  }

  if (!((Bus == 0) && (Device == 0) && (Function == 0))) {
    if (!CheckBdfValidity(Address)) {
      ConvAddress = (UINTN) &DummyPciData;
    }
  }

  return (VOID*)ConvAddress;
}

/**
  Reads an 8-bit PCI configuration register.

  Reads and returns the 8-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are
  serialized.

  If Address > 0x0FFFFFFF, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @return The read value from the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressRead8 (
  IN      UINTN                     Address
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioRead8 ((UINTN) GetPciExpressAddress (Address));
}

/**
  Writes an 8-bit PCI configuration register.

  Writes the 8-bit PCI configuration register specified by Address with the
  value specified by Value. Value is returned. This function must guarantee
  that all PCI read and write operations are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  Value   The value to write.

  @return The value written to the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressWrite8 (
  IN      UINTN                     Address,
  IN      UINT8                     Value
  )
{
  UINT32 Data;
  UINT8 Offset;
  UINT8 Bus;
  UINT8 Device;
  UINT8 Function;

  ASSERT_INVALID_PCI_ADDRESS (Address);

  // Get the EFI notation
  Bus = GET_BUS_NUM (Address);
  Device = GET_DEV_NUM (Address);
  Function = GET_FUNC_NUM (Address);

  if( (Bus == 0) && (Device == 0) && (Function == 0) ) {
    Data = MmioRead32 ((UINTN) GetPciExpressAddress (Address & 0xFFFFFFFC));
    Offset = Address & 0x3;
    Data |= (Value << (8 * Offset));
    MmioWrite32 ((UINTN) GetPciExpressAddress (Address & 0xFFFFFFFC), Data);
  } else {
    MmioWrite8 ((UINTN) GetPciExpressAddress (Address), Value);
  }
  return Value;
}

/**
  Performs a bitwise OR of an 8-bit PCI configuration register with
  an 8-bit value.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 8-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  OrData  The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressOr8 (
  IN      UINTN                     Address,
  IN      UINT8                     OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioOr8 ((UINTN) GetPciExpressAddress (Address), OrData);
}

/**
  Performs a bitwise AND of an 8-bit PCI configuration register with an 8-bit
  value.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 8-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized.

  If Address > 0x0FFFFFFF, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  AndData The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressAnd8 (
  IN      UINTN                     Address,
  IN      UINT8                     AndData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioAnd8 ((UINTN) GetPciExpressAddress (Address), AndData);
}

/**
  Performs a bitwise AND of an 8-bit PCI configuration register with an 8-bit
  value, followed a  bitwise OR with another 8-bit value.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and
  the value specified by OrData, and writes the result to the 8-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  AndData The value to AND with the PCI configuration register.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressAndThenOr8 (
  IN      UINTN                     Address,
  IN      UINT8                     AndData,
  IN      UINT8                     OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioAndThenOr8 (
           (UINTN) GetPciExpressAddress (Address),
           AndData,
           OrData
         );
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in an 8-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If Address > 0x0FFFFFFF, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.

  @return The value of the bit field read from the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressBitFieldRead8 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldRead8 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit
           );
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  8-bit register is returned.

  If Address > 0x0FFFFFFF, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  Value     The new value of the bit field.

  @return The value written back to the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressBitFieldWrite8 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT8                     Value
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldWrite8 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           Value
           );
}

/**
  Reads a bit field in an 8-bit PCI configuration, performs a bitwise OR, and
  writes the result back to the bit field in the 8-bit port.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 8-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized. Extra left bits in OrData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressBitFieldOr8 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT8                     OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldOr8 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           OrData
           );
}

/**
  Reads a bit field in an 8-bit PCI configuration register, performs a bitwise
  AND, and writes the result back to the bit field in the 8-bit register.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 8-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized. Extra left bits in AndData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressBitFieldAnd8 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT8                     AndData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldAnd8 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           AndData
           );
}

/**
  Reads a bit field in an 8-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  8-bit port.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 8-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.
**/
UINT8
EFIAPI
PciExpressBitFieldAndThenOr8 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT8                     AndData,
  IN      UINT8                     OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldAndThenOr8 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           AndData,
           OrData
           );
}

/**
  Reads a 16-bit PCI configuration register.

  Reads and returns the 16-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are
  serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @return The read value from the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressRead16 (
  IN      UINTN                     Address
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioRead16 ((UINTN) GetPciExpressAddress (Address));
}

/**
  Writes a 16-bit PCI configuration register.

  Writes the 16-bit PCI configuration register specified by Address with the
  value specified by Value. Value is returned. This function must guarantee
  that all PCI read and write operations are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  Value   The value to write.

  @return The value written to the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressWrite16 (
  IN      UINTN                     Address,
  IN      UINT16                    Value
  )
{
  UINT32 Data;
  UINT8 Offset;
  UINT8 Bus;
  UINT8 Device;
  UINT8 Function;

  ASSERT_INVALID_PCI_ADDRESS (Address);

  // Get the EFI notation
  Bus = GET_BUS_NUM (Address);
  Device = GET_DEV_NUM (Address);
  Function = GET_FUNC_NUM (Address);

  if( (Bus == 0) && (Device == 0) && (Function == 0) ) {
    Data = MmioRead32 ((UINTN) GetPciExpressAddress (Address & 0xFFFFFFFC));
    Offset = Address & 0x3;
    Data |= (Value << (8 * Offset));
    MmioWrite32 ((UINTN) GetPciExpressAddress (Address & 0xFFFFFFFC), Data);
  } else {
    MmioWrite16 ((UINTN) GetPciExpressAddress (Address), Value);
  }
  return Value;
}

/**
  Performs a bitwise OR of a 16-bit PCI configuration register with
  a 16-bit value.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 16-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  OrData  The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressOr16 (
  IN      UINTN                     Address,
  IN      UINT16                    OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioOr16 ((UINTN) GetPciExpressAddress (Address), OrData);
}

/**
  Performs a bitwise AND of a 16-bit PCI configuration register with a 16-bit
  value.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 16-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  AndData The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressAnd16 (
  IN      UINTN                     Address,
  IN      UINT16                    AndData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioAnd16 ((UINTN) GetPciExpressAddress (Address), AndData);
}

/**
  Performs a bitwise AND of a 16-bit PCI configuration register with a 16-bit
  value, followed a  bitwise OR with another 16-bit value.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and
  the value specified by OrData, and writes the result to the 16-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  AndData The value to AND with the PCI configuration register.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressAndThenOr16 (
  IN      UINTN                     Address,
  IN      UINT16                    AndData,
  IN      UINT16                    OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioAndThenOr16 (
           (UINTN) GetPciExpressAddress (Address),
           AndData,
           OrData
           );
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in a 16-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.

  @return The value of the bit field read from the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressBitFieldRead16 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldRead16 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit
           );
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  16-bit register is returned.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  Value     The new value of the bit field.

  @return The value written back to the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressBitFieldWrite16 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT16                    Value
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldWrite16 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           Value
           );
}

/**
  Reads a bit field in a 16-bit PCI configuration, performs a bitwise OR, and
  writes the result back to the bit field in the 16-bit port.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 16-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized. Extra left bits in OrData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressBitFieldOr16 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT16                    OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldOr16 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           OrData
           );
}

/**
  Reads a bit field in a 16-bit PCI configuration register, performs a bitwise
  AND, and writes the result back to the bit field in the 16-bit register.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 16-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized. Extra left bits in AndData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressBitFieldAnd16 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT16                    AndData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldAnd16 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           AndData
           );
}

/**
  Reads a bit field in a 16-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  16-bit port.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 16-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.
**/
UINT16
EFIAPI
PciExpressBitFieldAndThenOr16 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT16                    AndData,
  IN      UINT16                    OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldAndThenOr16 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           AndData,
           OrData
           );
}

/**
  Reads a 32-bit PCI configuration register.

  Reads and returns the 32-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are
  serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @return The read value from the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressRead32 (
  IN      UINTN                     Address
  )
{
  UINTN address;
  UINT32 value;
  ASSERT_INVALID_PCI_ADDRESS (Address);
  address = (UINTN) GetPciExpressAddress (Address);
  value = MmioRead32 (address);
  return value;
}

/**
  Writes a 32-bit PCI configuration register.

  Writes the 32-bit PCI configuration register specified by Address with the
  value specified by Value. Value is returned. This function must guarantee
  that all PCI read and write operations are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  Value   The value to write.

  @return The value written to the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressWrite32 (
  IN      UINTN                     Address,
  IN      UINT32                    Value
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioWrite32 ((UINTN) GetPciExpressAddress (Address), Value);
}

/**
  Performs a bitwise OR of a 32-bit PCI configuration register with
  a 32-bit value.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 32-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  OrData  The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressOr32 (
  IN      UINTN                     Address,
  IN      UINT32                    OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioOr32 ((UINTN) GetPciExpressAddress (Address), OrData);
}

/**
  Performs a bitwise AND of a 32-bit PCI configuration register with a 32-bit
  value.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 32-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  AndData The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressAnd32 (
  IN      UINTN                     Address,
  IN      UINT32                    AndData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioAnd32 ((UINTN) GetPciExpressAddress (Address), AndData);
}

/**
  Performs a bitwise AND of a 32-bit PCI configuration register with a 32-bit
  value, followed a  bitwise OR with another 32-bit value.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and
  the value specified by OrData, and writes the result to the 32-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  AndData The value to AND with the PCI configuration register.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressAndThenOr32 (
  IN      UINTN                     Address,
  IN      UINT32                    AndData,
  IN      UINT32                    OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioAndThenOr32 (
           (UINTN) GetPciExpressAddress (Address),
           AndData,
           OrData
           );
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in a 32-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.

  @return The value of the bit field read from the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressBitFieldRead32 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldRead32 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit
           );
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  32-bit register is returned.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  Value     The new value of the bit field.

  @return The value written back to the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressBitFieldWrite32 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT32                    Value
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldWrite32 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           Value
           );
}

/**
  Reads a bit field in a 32-bit PCI configuration, performs a bitwise OR, and
  writes the result back to the bit field in the 32-bit port.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 32-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized. Extra left bits in OrData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressBitFieldOr32 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT32                    OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldOr32 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           OrData
           );
}

/**
  Reads a bit field in a 32-bit PCI configuration register, performs a bitwise
  AND, and writes the result back to the bit field in the 32-bit register.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 32-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized. Extra left bits in AndData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressBitFieldAnd32 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT32                    AndData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldAnd32 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           AndData
           );
}

/**
  Reads a bit field in a 32-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  32-bit port.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 32-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If Address > 0x0FFFFFFF, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.
**/
UINT32
EFIAPI
PciExpressBitFieldAndThenOr32 (
  IN      UINTN                     Address,
  IN      UINTN                     StartBit,
  IN      UINTN                     EndBit,
  IN      UINT32                    AndData,
  IN      UINT32                    OrData
  )
{
  ASSERT_INVALID_PCI_ADDRESS (Address);
  return MmioBitFieldAndThenOr32 (
           (UINTN) GetPciExpressAddress (Address),
           StartBit,
           EndBit,
           AndData,
           OrData
           );
}

/**
  Reads a range of PCI configuration registers into a caller supplied buffer.

  Reads the range of PCI configuration registers specified by StartAddress and
  Size into the buffer specified by Buffer. This function only allows the PCI
  configuration registers from a single PCI function to be read. Size is
  returned. When possible 32-bit PCI configuration read cycles are used to read
  from StartAdress to StartAddress + Size. Due to alignment restrictions, 8-bit
  and 16-bit PCI configuration read cycles may be used at the beginning and the
  end of the range.

  If StartAddress > 0x0FFFFFFF, then ASSERT().
  If ((StartAddress & 0xFFF) + Size) > 0x1000, then ASSERT().
  If Size > 0 and Buffer is NULL, then ASSERT().

  @param  StartAddress  The starting address that encodes the PCI Bus, Device,
                        Function and Register.
  @param  Size          The size in bytes of the transfer.
  @param  Buffer        The pointer to a buffer receiving the data read.

  @return Size read data from StartAddress.
**/
UINTN
EFIAPI
PciExpressReadBuffer (
  IN      UINTN                     StartAddress,
  IN      UINTN                     Size,
  OUT     VOID                      *Buffer
  )
{
  UINTN   ReturnValue;

  ASSERT_INVALID_PCI_ADDRESS (StartAddress);
  ASSERT (((StartAddress & 0xFFF) + Size) <= 0x1000);

  if (Size == 0) {
    return Size;
  }

  ASSERT (Buffer != NULL);

  //
  // Save Size for return
  //
  ReturnValue = Size;

  if ((StartAddress & 1) != 0) {
    //
    // Read a byte if StartAddress is byte aligned
    //
    *(volatile UINT8 *)Buffer = PciExpressRead8 (StartAddress);
    StartAddress += sizeof (UINT8);
    Size -= sizeof (UINT8);
    Buffer = (UINT8*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16) && (StartAddress & 2) != 0) {
    //
    // Read a word if StartAddress is word aligned
    //
    WriteUnaligned16 ((UINT16 *) Buffer, (UINT16) PciExpressRead16 (StartAddress));

    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  while (Size >= sizeof (UINT32)) {
    //
    // Read as many double words as possible
    //
    WriteUnaligned32 ((UINT32 *) Buffer, (UINT32) PciExpressRead32 (StartAddress));

    StartAddress += sizeof (UINT32);
    Size -= sizeof (UINT32);
    Buffer = (UINT32*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16)) {
    //
    // Read the last remaining word if exist
    //
    WriteUnaligned16 ((UINT16 *) Buffer, (UINT16) PciExpressRead16 (StartAddress));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  if (Size >= sizeof (UINT8)) {
    //
    // Read the last remaining byte if exist
    //
    *(volatile UINT8 *)Buffer = PciExpressRead8 (StartAddress);
  }

  return ReturnValue;
}

/**
  Copies the data in a caller supplied buffer to a specified range of PCI
  configuration space.

  Writes the range of PCI configuration registers specified by StartAddress and
  Size from the buffer specified by Buffer. This function only allows the PCI
  configuration registers from a single PCI function to be written. Size is
  returned. When possible 32-bit PCI configuration write cycles are used to
  write from StartAdress to StartAddress + Size. Due to alignment restrictions,
  8-bit and 16-bit PCI configuration write cycles may be used at the beginning
  and the end of the range.

  If StartAddress > 0x0FFFFFFF, then ASSERT().
  If ((StartAddress & 0xFFF) + Size) > 0x1000, then ASSERT().
  If Size > 0 and Buffer is NULL, then ASSERT().

  @param  StartAddress  The starting address that encodes the PCI Bus, Device,
                        Function and Register.
  @param  Size          The size in bytes of the transfer.
  @param  Buffer        The pointer to a buffer containing the data to write.

  @return Size written to StartAddress.
**/
UINTN
EFIAPI
PciExpressWriteBuffer (
  IN      UINTN                     StartAddress,
  IN      UINTN                     Size,
  IN      VOID                      *Buffer
  )
{
  UINTN                             ReturnValue;

  ASSERT_INVALID_PCI_ADDRESS (StartAddress);
  ASSERT (((StartAddress & 0xFFF) + Size) <= 0x1000);

  if (Size == 0) {
    return 0;
  }

  ASSERT (Buffer != NULL);

  //
  // Save Size for return
  //
  ReturnValue = Size;

  if ((StartAddress & 1) != 0) {
    //
    // Write a byte if StartAddress is byte aligned
    //
    PciExpressWrite8 (StartAddress, *(UINT8*)Buffer);
    StartAddress += sizeof (UINT8);
    Size -= sizeof (UINT8);
    Buffer = (UINT8*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16) && (StartAddress & 2) != 0) {
    //
    // Write a word if StartAddress is word aligned
    //
    PciExpressWrite16 (StartAddress, ReadUnaligned16 ((UINT16*)Buffer));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  while (Size >= sizeof (UINT32)) {
    //
    // Write as many double words as possible
    //
    PciExpressWrite32 (StartAddress, ReadUnaligned32 ((UINT32*)Buffer));
    StartAddress += sizeof (UINT32);
    Size -= sizeof (UINT32);
    Buffer = (UINT32*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16)) {
    //
    // Write the last remaining word if exist
    //
    PciExpressWrite16 (StartAddress, ReadUnaligned16 ((UINT16*)Buffer));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  if (Size >= sizeof (UINT8)) {
    //
    // Write the last remaining byte if exist
    //
    PciExpressWrite8 (StartAddress, *(UINT8*)Buffer);
  }

  return ReturnValue;
}
