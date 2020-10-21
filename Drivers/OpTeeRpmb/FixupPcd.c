/** @file

  Update the patched PCDs to their correct value

  Copyright (c) 2020, Linaro Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/**
 * Patch the relevant PCDs of the RPMB driver with the correct address of the
 * allocated memory
 *
**/
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MmServicesTableLib.h>
#include <Library/PcdLib.h>

#include <Protocol/FirmwareVolumeBlock.h>
#include <Protocol/SmmFirmwareVolumeBlock.h>

#include "OpTeeRpmbFvb.h"

/**
  Fixup the Pcd values for variable storage

  Since the upper layers of EDK2 expect a memory mapped interface and we can't
  offer that from an RPMB, the driver allocates memory on init and passes that
  on the upper layers. Since the memory is dynamically allocated and we can't set the
  PCD is StMM context, we need to patch it correctly on each access

  @retval EFI_SUCCESS Protocol was found and PCDs patched up

 **/
EFI_STATUS
EFIAPI
FixPcdMemory (
  VOID
  )
{
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  *FvbProtocol;
  MEM_INSTANCE                        *Instance;
  EFI_STATUS                          Status;

  //
  // Locate SmmFirmwareVolumeBlockProtocol
  //
  Status = gMmst->MmLocateProtocol (
                    &gEfiSmmFirmwareVolumeBlockProtocolGuid,
                    NULL,
                    (VOID **) &FvbProtocol
                    );
  ASSERT_EFI_ERROR (Status);

  Instance = INSTANCE_FROM_FVB_THIS(FvbProtocol);
  // Patch PCDs with the the correct values
  PatchPcdSet32 (PcdFlashNvStorageVariableBase, Instance->MemBaseAddress);
  PatchPcdSet32 (PcdFlashNvStorageFtwWorkingBase, Instance->MemBaseAddress +
    PcdGet32 (PcdFlashNvStorageVariableSize));
  PatchPcdSet32 (PcdFlashNvStorageFtwSpareBase, Instance->MemBaseAddress +
    PcdGet32 (PcdFlashNvStorageVariableSize) +
    PcdGet32 (PcdFlashNvStorageFtwWorkingSize));

  DEBUG ((DEBUG_INFO, "%a: Fixup PcdFlashNvStorageVariableBase: 0x%lx\n",
    __FUNCTION__, PcdGet32 (PcdFlashNvStorageVariableBase)));
  DEBUG ((DEBUG_INFO, "%a: Fixup PcdFlashNvStorageFtwWorkingBase: 0x%lx\n",
    __FUNCTION__, PcdGet32 (PcdFlashNvStorageFtwWorkingBase)));
  DEBUG ((DEBUG_INFO, "%a: Fixup PcdFlashNvStorageFtwSpareBase: 0x%lx\n",
    __FUNCTION__, PcdGet32 (PcdFlashNvStorageFtwSpareBase)));

  return Status;
}
