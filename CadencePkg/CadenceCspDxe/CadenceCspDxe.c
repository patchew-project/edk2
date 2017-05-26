/** @file

  Copyright (c) 2013-2015, ARM Ltd. All rights reserved.<BR>
  Copyright (c) 2017, Cadence Design Systems, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmShellCmdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/FirmwareVolume2.h>

/**
 * Generic UEFI Entrypoint for 'CadenceCspDxe' driver
 * See UEFI specification for the details of the parameters
 */
EFI_STATUS
EFIAPI
CadenceCspInitialise (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                   Status;

  // Install dynamic Shell command to run baremetal binaries.
  Status = ShellDynCmdRunAxfInstall (ImageHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "CadenceCspDxe: Failed to install ShellDynCmdRunAxf\n"));
  }

  return Status;
}
