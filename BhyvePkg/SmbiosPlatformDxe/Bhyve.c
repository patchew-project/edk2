/*
 * Copyright (c) 2014, Pluribus Networks, Inc.
 *
 * This program and the accompanying materials are licensed and made
 * available under the terms and conditions of the BSD License which
 * accompanies this distribution.  The full text of the license may be
 * found at http://opensource.org/licenses/bsd-license.php
 *
 * THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
 * BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED.
 */

#include "SmbiosPlatformDxe.h"

#define BHYVE_SMBIOS_PHYSICAL_ADDRESS       0x000F0000
#define BHYVE_SMBIOS_PHYSICAL_END           0x000FFFFF

/**
  Locates the bhyve SMBIOS data if it exists

  @return SMBIOS_TABLE_ENTRY_POINT   Address of bhyve SMBIOS data

**/
SMBIOS_TABLE_ENTRY_POINT *
GetBhyveSmbiosTables (
  VOID
  )
{
  UINT8                     *BhyveSmbiosPtr;
  SMBIOS_TABLE_ENTRY_POINT  *BhyveSmbiosEntryPointStructure;

  for (BhyveSmbiosPtr = (UINT8*)(UINTN) BHYVE_SMBIOS_PHYSICAL_ADDRESS;
       BhyveSmbiosPtr < (UINT8*)(UINTN) BHYVE_SMBIOS_PHYSICAL_END;
       BhyveSmbiosPtr += 0x10) {

    BhyveSmbiosEntryPointStructure = (SMBIOS_TABLE_ENTRY_POINT *) BhyveSmbiosPtr;

    if (!AsciiStrnCmp ((CHAR8 *) BhyveSmbiosEntryPointStructure->AnchorString, "_SM_", 4) &&
        !AsciiStrnCmp ((CHAR8 *) BhyveSmbiosEntryPointStructure->IntermediateAnchorString, "_DMI_", 5) &&
        IsEntryPointStructureValid (BhyveSmbiosEntryPointStructure)) {

      return BhyveSmbiosEntryPointStructure;

    }
  }

  return NULL;
}
