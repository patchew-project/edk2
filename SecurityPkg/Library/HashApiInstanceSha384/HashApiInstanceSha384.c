/** @file
  This library is BaseCrypto SHA384 hash instance.
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
Sha384_Init (
  OUT HASH_HANDLE    *HashHandle
  )
{
  VOID     *Sha384Ctx;
  UINTN    CtxSize;

  CtxSize = Sha384GetContextSize ();
  Sha384Ctx = AllocatePool (CtxSize);
  ASSERT (Sha384Ctx != NULL);

  Sha384Init (Sha384Ctx);

  *HashHandle = (HASH_HANDLE)Sha384Ctx;

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
Sha384_Update (
  IN HASH_HANDLE    HashHandle,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
  )
{
  VOID     *Sha384Ctx;

  Sha384Ctx = (VOID *)HashHandle;
  Sha384Update (Sha384Ctx, DataToHash, DataToHashLen);

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
Sha384_Final (
  IN HASH_HANDLE         HashHandle,
  OUT UINT8            **Digest
  )
{
  UINT8         Sha384Digest[SHA384_DIGEST_SIZE];
  VOID          *Sha384Ctx;

  Sha384Ctx = (VOID *)HashHandle;
  Sha384Final (Sha384Ctx, Sha384Digest);

  CopyMem (*Digest, Sha384Digest, SHA384_DIGEST_SIZE);

  FreePool (Sha384Ctx);

  return EFI_SUCCESS;
}

HASH_INTERFACE_UNIFIED_API  mSha384InternalHashApiInstance = {
  HASH_ALGORITHM_SHA384_GUID,
  Sha384_Init,
  Sha384_Update,
  Sha384_Final,
};

/**
  The function register SHA384 instance.

  @retval EFI_SUCCESS   SHA384 instance is registered, or system dose not surpport registr SHA384 instance
**/
EFI_STATUS
EFIAPI
HashApiInstanceSha384Constructor (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = RegisterHashApiLib (&mSha384InternalHashApiInstance);
  if ((Status == EFI_SUCCESS) || (Status == EFI_UNSUPPORTED)) {
    //
    // Unsupported means platform policy does not need this instance enabled.
    //
    DEBUG ((DEBUG_ERROR, "[ansukerk]: Hash Interface SHA384 is registered\n"));
    return EFI_SUCCESS;
  }
  return Status;
}
