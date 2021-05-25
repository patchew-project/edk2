/** @file
  AMD SEV Firmware Config file verifier

  Copyright (C) 2021 James Bottomley, IBM Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/MemEncryptSevLib.h>
#include <Library/QemuFwCfgLib.h>
#include <Library/SevHashFinderLib.h>

STATIC EFI_STATUS
EFIAPI
SevCmdLineVerifier (
  IN  CONST CHAR16    *Name,
  IN  VOID            *Buffer,
  IN  UINTN           Size
  )
{
  DEBUG ((DEBUG_INFO, "%a: Validating Hash\n", __FUNCTION__));

  return ValidateHashEntry (&SEV_CMDLINE_HASH_GUID, Buffer, Size);
}

/**
  Register security measurement handler.

  @param  ImageHandle   ImageHandle of the loaded driver.
  @param  SystemTable   Pointer to the EFI System Table.

  @retval EFI_SUCCESS   The handlers were registered successfully.
**/
EFI_STATUS
EFIAPI
SevQemuLoadImageConstructor (
  VOID
  )
{
  if (MemEncryptSevIsEnabled ()) {
    DEBUG ((DEBUG_INFO, "Enabling hash verification of fw_cfg cmdline\n"));
    return RegisterFwCfgVerifier (SevCmdLineVerifier);
  } else {
    //
    // Don't install verifier if SEV isn't enabled
    //
    DEBUG ((DEBUG_INFO, "NOT Enabling hash verification of fw_cfg cmdline\n"));
    return EFI_SUCCESS;
  }
}
