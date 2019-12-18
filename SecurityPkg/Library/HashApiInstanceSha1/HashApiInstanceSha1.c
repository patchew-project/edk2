/** @file
  This library is BaseCrypto SHA1 hash instance.
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
Sha1_Init (
  OUT HASH_HANDLE    *HashHandle
  )
{
  VOID     *Sha1Ctx;
  UINTN    CtxSize;

  CtxSize = Sha1GetContextSize ();
  Sha1Ctx = AllocatePool (CtxSize);
  ASSERT (Sha1Ctx != NULL);

  Sha1Init (Sha1Ctx);

  *HashHandle = (HASH_HANDLE)Sha1Ctx;

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
Sha1_Update (
  IN HASH_HANDLE    HashHandle,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
  )
{
  VOID     *Sha1Ctx;

  Sha1Ctx = (VOID *)HashHandle;
  Sha1Update (Sha1Ctx, DataToHash, DataToHashLen);

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
Sha1_Final (
  IN HASH_HANDLE         HashHandle,
  OUT UINT8            **Digest
  )
{
  UINT8         Sha1Digest[SHA1_DIGEST_SIZE];
  VOID          *Sha1Ctx;

  Sha1Ctx = (VOID *)HashHandle;
  Sha1Final (Sha1Ctx, Sha1Digest);

  CopyMem (*Digest, Sha1Digest, SHA1_DIGEST_SIZE);

  FreePool (Sha1Ctx);

  return EFI_SUCCESS;
}

HASH_INTERFACE_UNIFIED_API  mSha1InternalHashApiInstance = {
  HASH_ALGORITHM_SHA1_GUID,
  Sha1_Init,
  Sha1_Update,
  Sha1_Final,
};

/**
  The function register SHA1 instance.

  @retval EFI_SUCCESS   SHA1 instance is registered, or system dose not surpport registr SHA1 instance
**/
EFI_STATUS
EFIAPI
HashApiInstanceSha1Constructor (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = RegisterHashApiLib (&mSha1InternalHashApiInstance);
  if ((Status == EFI_SUCCESS) || (Status == EFI_UNSUPPORTED)) {
    //
    // Unsupported means platform policy does not need this instance enabled.
    //
    DEBUG ((DEBUG_ERROR, "[ansukerk]: Hash Interface SHA1 is registered\n"));
    return EFI_SUCCESS;
  }
  return Status;
}
