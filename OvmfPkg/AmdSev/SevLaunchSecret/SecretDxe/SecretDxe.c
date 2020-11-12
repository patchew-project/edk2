/** @file
  SEV Secret configuration table constructor

  Copyright (C) 2020 James Bottomley, IBM Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

struct {
  UINT32        base;
  UINT32        size;
} secretDxeTable = {
  FixedPcdGet32(PcdSevLaunchSecretBase),
  FixedPcdGet32(PcdSevLaunchSecretSize),
};

EFI_STATUS
EFIAPI
InitializeSecretDxe(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  return gBS->InstallConfigurationTable (&gSevLaunchSecretGuid,
                                         &secretDxeTable);
}
