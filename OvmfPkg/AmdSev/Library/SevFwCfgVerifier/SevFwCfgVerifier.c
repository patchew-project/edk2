/** @file
  AMD SEV Firmware Config file verifier

  Copyright (C) 2021 James Bottomley, IBM Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemEncryptSevLib.h>
#include <Library/QemuFwCfgLib.h>
#include <Library/SevHashFinderLib.h>

STATIC EFI_STATUS
EFIAPI
SevFwCfgVerifier (
  IN  CONST CHAR16    *Name,
  IN  VOID            *Buffer,
  IN  UINTN           Size
  )
{
  DEBUG ((DEBUG_INFO, "%a: Validating Hash of %s\n", __FUNCTION__, Name));

  if (StrCmp (Name, L"kernel") == 0) {
    return ValidateHashEntry (&SEV_KERNEL_HASH_GUID, Buffer, Size);
  }
  if (StrCmp (Name, L"initrd") == 0) {
    return ValidateHashEntry (&SEV_INITRD_HASH_GUID, Buffer, Size);
  }

  DEBUG ((DEBUG_ERROR, "%a: Failed to find Filename %s", __FUNCTION__, Name));
  return EFI_SECURITY_VIOLATION;
}

/**
  Register security measurement handler.

  @param  ImageHandle   ImageHandle of the loaded driver.
  @param  SystemTable   Pointer to the EFI System Table.

  @retval EFI_SUCCESS   The handlers were registered successfully.
**/
EFI_STATUS
EFIAPI
SevFwCfgVerifierConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (MemEncryptSevIsEnabled ()) {
    DEBUG ((DEBUG_INFO, "Enabling hash verification of fw_cfg files"));
    return RegisterFwCfgVerifier (SevFwCfgVerifier);
  } else {
    //
    // Don't install verifier if SEV isn't enabled
    //
    DEBUG ((DEBUG_INFO, "NOT Enabling hash verification of fw_cfg files"));
    return EFI_SUCCESS;
  }
}
