/** ResetSystemLib.c
  Do a generic Cold Reset

  Based on Reset system library implementation in
  BeagleBoardPkg/Library/ResetSystemLib/ResetSystemLib.c

  Copyright (c) 2008 - 2010, Apple Inc. All rights reserved.<BR>
  Copyright (c) 2016, Freescale Semiconductor, Inc. All rights reserved.
  Copyright 2017 NXP

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/UefiLib.h>

/**
  Resets the entire platform.

  @param  ResetType     :     The type of reset to perform.
  @param  ResetStatus   :     The status code for the reset.
  @param  DataSize      :     The size, in bytes, of WatchdogData.
  @param  ResetData     :     For a ResetType of EfiResetCold, EfiResetWarm, or
                              EfiResetShutdown the data buffer starts with a Null-terminated
                              Unicode string, optionally followed by additional binary data.
**/
EFI_STATUS
EFIAPI
LibResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  )
{
  ARM_SMC_ARGS ArmSmcArgs;

  switch (ResetType) {
  case EfiResetPlatformSpecific:
  case EfiResetWarm:
    // Map a warm reset into a cold reset
  case EfiResetCold:
    // Send a PSCI 0.2 SYSTEM_RESET command
    ArmSmcArgs.Arg0 = ARM_SMC_ID_PSCI_SYSTEM_RESET;
    break;
  case EfiResetShutdown:
    // Send a PSCI 0.2 SYSTEM_OFF command
    ArmSmcArgs.Arg0 = ARM_SMC_ID_PSCI_SYSTEM_OFF;
    break;
  default:
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  ArmCallSmc (&ArmSmcArgs);

  // We should never be here
  DEBUG ((DEBUG_VERBOSE, "%a: PSCI failed in performing %d\n",
                            __FUNCTION__, ArmSmcArgs.Arg0));

  CpuDeadLoop ();
  return EFI_UNSUPPORTED;
}

/**
  Initialize any infrastructure required for LibResetSystem () to function.

  @param  ImageHandle  :  The firmware allocated handle for the EFI image.
  @param  SystemTable  :  A pointer to the EFI System Table.

  @retval EFI_SUCCESS  :  The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
LibInitializeResetSystem (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
