/** @file

  Extends one of the RTMR measurement registers in TDCS with the provided
  extension data in memory.

  Copyright (c) 2020 - 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/TdxLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/Tdx.h>
#include <Protocol/Tdx.h>

#define RTMR_COUNT  4

/**
  This function extends one of the RTMR measurement register
  in TDCS with the provided extension data in memory.
  RTMR extending supports SHA384 which length is 48 bytes.

  @param[in]  Data      Point to the data to be extended
  @param[in]  DataLen   Length of the data. Must be 48
  @param[in]  Index     RTMR index

  @return EFI_SUCCESS
  @return EFI_INVALID_PARAMETER
  @return EFI_DEVICE_ERROR

**/
EFI_STATUS
EFIAPI
TdExtendRtmr(
  IN  UINT32  *Data,
  IN  UINT32  DataLen,
  IN  UINT8   Index
  )
{
  EFI_STATUS            Status;
  UINT64                *Buffer;
  UINT64                TdCallStatus;

  Status = EFI_SUCCESS;

  ASSERT(Index >= 0 && Index < RTMR_COUNT);
  ASSERT(DataLen == SHA384_DIGEST_SIZE);

  //
  // Allocate 64B aligned mem to hold the sha384 hash value
  //
  Buffer = AllocateAlignedPages(EFI_SIZE_TO_PAGES(SHA384_DIGEST_SIZE), 64);
  if(Data == NULL){
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem(Buffer, Data, SHA384_DIGEST_SIZE);

  TdCallStatus = TdCall(TDCALL_TDEXTENDRTMR, (UINT64)Buffer, Index, 0, 0);

  if(TdCallStatus == TDX_EXIT_REASON_SUCCESS){
    Status = EFI_SUCCESS;
  }else if(TdCallStatus == TDX_EXIT_REASON_OPERAND_INVALID){
    Status = EFI_INVALID_PARAMETER;
  }else{
    Status = EFI_DEVICE_ERROR;
  }

  if(Status != EFI_SUCCESS){
    DEBUG((DEBUG_ERROR, "Error returned from TdExtendRtmr call - 0x%lx\n", TdCallStatus));
  }

  FreeAlignedPages(Buffer, EFI_SIZE_TO_PAGES(SHA384_DIGEST_SIZE));

  return Status;
}
