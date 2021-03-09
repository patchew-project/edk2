/** @file

  There are 4 defined types in TD memory.
  Unaccepted memory is a special type of private memory. The OVMF must
  invoke TDCALL [TDG.MEM.PAGE.ACCEPT] the unaccepted memory before use it.

  Copyright (c) 2020 - 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>
#include <Library/BaseMemoryLib.h>

UINT64  mNumberOfDuplicatedAcceptedPages;

/**
  This function accept a pending private page, and initialize the page to
  all-0 using the TD ephemeral private key.

  @param[in]  StartAddress           Guest physical address of the private
                                     page to accept.
  @param[in]  NumberOfPages          Number of the pages to be accepted.

  @return EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
TdAcceptPages (
  IN UINT64  StartAddress,
  IN UINT64  NumberOfPages
  )
{
  UINT64  Address;
  UINT64  Status;
  UINT64  Index;

  //
  // Determine if we need to accept pages before use
  //
  if (FixedPcdGetBool(PcdUseTdxAcceptPage) == FALSE) {
     return EFI_SUCCESS;
  }

  Address = StartAddress;

  for( Index = 0; Index < NumberOfPages; Index++) {
    Status = TdCall(TDCALL_TDACCEPTPAGE,Address, 0, 0, 0);
    if (Status != TDX_EXIT_REASON_SUCCESS) {
        if ((Status & ~0xFFULL) == TDX_EXIT_REASON_PAGE_ALREADY_ACCEPTED) {
          ++mNumberOfDuplicatedAcceptedPages;
          DEBUG((DEBUG_VERBOSE, "Address %llx already accepted. Total number of already accepted pages %ld\n",
            Address, mNumberOfDuplicatedAcceptedPages));
        } else {
          DEBUG((DEBUG_ERROR, "Address %llx failed to be accepted. Error = %ld\n",
            Address, Status));
          ASSERT(Status == TDX_EXIT_REASON_SUCCESS);
        }
    }
    Address += EFI_PAGE_SIZE;
  }
  return EFI_SUCCESS;
}

