/** @file
  Provides interface to performs a CPU architecture specific operations to
  transit to DXE phase.

  Copyright (c) 2020, Hewlett Packard Enterprise Development LP. <BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef DXEIPL_HANDOFF_LIB__
#define DXEIPL_HANDOFF_LIB__

#include <PiPei.h>

/**
   Transfers control to DxeCore.

   This function performs a CPU architecture specific operations to execute
   the entry point of DxeCore with the parameters of HobList.
   It also installs EFI_END_OF_PEI_PPI to signal the end of PEI phase.

   @param DxeCoreEntryPoint         The entry point of DxeCore.
   @param HobList                   The start of HobList passed to DxeCore.

**/
VOID
EFIAPI
HandOffToDxeCore (
  IN EFI_PHYSICAL_ADDRESS   DxeCoreEntryPoint,
  IN EFI_PEI_HOB_POINTERS   HobList
  );

#endif
