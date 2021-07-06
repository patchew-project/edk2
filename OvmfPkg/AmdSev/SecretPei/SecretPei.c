/** @file
  SEV Secret boot time HOB placement

  Copyright (C) 2020 James Bottomley, IBM Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <PiPei.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>

EFI_STATUS
EFIAPI
InitializeSecretPei (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  UINT64 RoundedSize;

  RoundedSize = PcdGet32 (PcdSevLaunchSecretSize);
  if (RoundedSize % EFI_PAGE_SIZE != 0) {
    RoundedSize = (RoundedSize / EFI_PAGE_SIZE + 1) * EFI_PAGE_SIZE;
  }

  BuildMemoryAllocationHob (
    PcdGet32 (PcdSevLaunchSecretBase),
    RoundedSize,
    EfiBootServicesData
    );

  return EFI_SUCCESS;
}
