/** @file
  This library is BaseCrypto Sm3 hash instance.
  It can be registered to BaseCrypto router, to serve as hash engine.

Copyright (c) 2013 - 2019, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseHashLib.h>

/**
  Start hash sequence.

  @param HashHandle Hash handle.

  @retval EFI_SUCCESS          Hash sequence start and HandleHandle returned.
  @retval EFI_OUT_OF_RESOURCES No enough resource to start hash.
**/
EFI_STATUS
EFIAPI
Sm3_Init (
  OUT HASH_HANDLE    *HashHandle
  )
{
  VOID     *Sm3Ctx;
  UINTN    CtxSize;

  CtxSize = Sm3GetContextSize ();
  Sm3Ctx = AllocatePool (CtxSize);
  ASSERT (Sm3Ctx != NULL);

  Sm3Init (Sm3Ctx);

  *HashHandle = (HASH_HANDLE)Sm3Ctx;

  return EFI_SUCCESS;
}

/**
  Update hash sequence data.

  @param HashHandle    Hash handle.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.

  @retval EFI_SUCCESS     Hash sequence updated.
**/
EFI_STATUS
EFIAPI
Sm3_Update (
  IN HASH_HANDLE    HashHandle,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
  )
{
  VOID     *Sm3Ctx;

  Sm3Ctx = (VOID *)HashHandle;
  Sm3Update (Sm3Ctx, DataToHash, DataToHashLen);

  return EFI_SUCCESS;
}

/**
  Complete hash sequence complete.

  @param HashHandle    Hash handle.
  @param DigestList    Digest list.

  @retval EFI_SUCCESS     Hash sequence complete and DigestList is returned.
**/
EFI_STATUS
EFIAPI
Sm3_Final (
  IN HASH_HANDLE         HashHandle,
  OUT UINT8            **Digest
  )
{
  UINT8         Sm3Digest[SM3_256_DIGEST_SIZE];
  VOID          *Sm3Ctx;

  Sm3Ctx = (VOID *)HashHandle;
  Sm3Final (Sm3Ctx, Sm3Digest);

  CopyMem (*Digest, Sm3Digest, SM3_256_DIGEST_SIZE);

  FreePool (Sm3Ctx);

  return EFI_SUCCESS;
}

HASH_INTERFACE_UNIFIED_API  mSm3InternalHashApiInstance = {
  HASH_ALGORITHM_SM3_256_GUID,
  Sm3_Init,
  Sm3_Update,
  Sm3_Final,
};

/**
  The function register Sm3 instance.

  @retval EFI_SUCCESS   Sm3 instance is registered, or system dose not surpport registr Sm3 instance
**/
EFI_STATUS
EFIAPI
HashApiInstanceSm3Constructor (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = RegisterHashApiLib (&mSm3InternalHashApiInstance);
  if ((Status == EFI_SUCCESS) || (Status == EFI_UNSUPPORTED)) {
    //
    // Unsupported means platform policy does not need this instance enabled.
    //
    DEBUG ((DEBUG_ERROR, "[ansukerk]: Hash Interface Sm3 is registered\n"));
    return EFI_SUCCESS;
  }
  return Status;
}
