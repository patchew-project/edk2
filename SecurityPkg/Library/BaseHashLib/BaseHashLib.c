/** @file
  Implement image verification services for secure boot service

  Caution: This file requires additional review when modified.
  This library will have external input - PE/COFF image.
  This external input must be validated carefully to avoid security issue like
  buffer overflow, integer overflow.

  DxeImageVerificationLibImageRead() function will make sure the PE/COFF image content
  read is within the image buffer.

  DxeImageVerificationHandler(), HashPeImageByType(), HashPeImage() function will accept
  untrusted PE/COFF image and validate its data structure within this image buffer before use.

Copyright (c) 2009 - 2019, Intel Corporation. All rights reserved.<BR>
(C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/HashLib.h>

//#include "BaseHashLib.h"

typedef struct {
  EFI_GUID  Guid;
  UINT32    Mask;
} HASH_MASK;

HASH_MASK mHashMask[] = {
  {HASH_ALGORITHM_SHA1_GUID,         HASH_ALG_SHA1},
  {HASH_ALGORITHM_SHA256_GUID,       HASH_ALG_SHA256},
  {HASH_ALGORITHM_SHA384_GUID,       HASH_ALG_SHA384},
  {HASH_ALGORITHM_SHA512_GUID,       HASH_ALG_SHA512},
};

HASH_INTERFACE_UNIFIED_API mHashOps[HASH_COUNT] = {{{0}, NULL, NULL, NULL}};

UINTN mHashInterfaceCount = 0;
UINT32 mCurrentHashMask = 0;

UINT32
EFIAPI
GetApiHashMaskFromAlgo (
  IN EFI_GUID  *HashGuid
  )
{
  UINTN  Index;

  for (Index = 0; Index < sizeof(mHashMask)/sizeof(mHashMask[0]); Index++) {
    if (CompareGuid (HashGuid, &mHashMask[Index].Guid)) {
      return mHashMask[Index].Mask;
    }
  }
  return 0;
}

/**
  Init hash sequence.

  @param HashHandle Hash handle.

  @retval EFI_SUCCESS          Hash start and HashHandle returned.
  @retval EFI_UNSUPPORTED      System has no HASH library registered.
**/
EFI_STATUS
EFIAPI
HashApiInit (
  OUT  HASH_HANDLE   *HashHandle
)
{
  HASH_HANDLE    *HashCtx;
  UINTN   Index;
  UINT32  HashMask;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = AllocatePool (sizeof(*HashCtx));
  ASSERT (HashCtx != NULL);

  for (Index = 0; Index < mHashInterfaceCount; Index++) {
    HashMask = GetApiHashMaskFromAlgo (&mHashOps[Index].HashGuid);
    if ((HashMask & PcdGet32 (PcdHashAlgorithmBitmap)) != 0 &&
        (HashMask & PcdGet32 (PcdSystemHashPolicy)) != 0) {
      mHashOps[Index].HashInit (HashCtx);
    }
  }

  *HashHandle = (HASH_HANDLE)HashCtx;

  return EFI_SUCCESS;
}

/**
  Update hash data.

  @param HashHandle    Hash handle.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.

  @retval EFI_SUCCESS          Hash updated.
  @retval EFI_UNSUPPORTED      System has no HASH library registered.
**/
EFI_STATUS
EFIAPI
HashApiUpdate (
  IN HASH_HANDLE    HashHandle,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
)
{
  HASH_HANDLE  *HashCtx;
  UINTN        Index;
  UINT32       HashMask;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = (HASH_HANDLE *)HashHandle;

  for (Index = 0; Index < mHashInterfaceCount; Index++) {
    HashMask = GetApiHashMaskFromAlgo (&mHashOps[Index].HashGuid);
    if ((HashMask & PcdGet32 (PcdHashAlgorithmBitmap)) != 0 &&
        (HashMask & PcdGet32 (PcdSystemHashPolicy)) != 0) {
      mHashOps[Index].HashUpdate (HashCtx[0], DataToHash, DataToHashLen);
    }
  }

  return EFI_SUCCESS;
}

/**
  Hash complete.

  @param HashHandle    Hash handle.
  @param Digest        Hash Digest.

  @retval EFI_SUCCESS     Hash complete and Digest is returned.
**/
EFI_STATUS
EFIAPI
HashApiFinal (
  IN  HASH_HANDLE HashHandle,
  OUT UINT8       *Digest
)
{
  HASH_HANDLE  *HashCtx;
  UINTN        Index;
  UINT32       HashMask;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = (HASH_HANDLE *)HashHandle;

  for (Index = 0; Index < mHashInterfaceCount; Index++) {
    HashMask = GetApiHashMaskFromAlgo (&mHashOps[Index].HashGuid);
    if ((HashMask & PcdGet32 (PcdHashAlgorithmBitmap)) != 0 &&
        (HashMask & PcdGet32 (PcdSystemHashPolicy)) != 0) {
      mHashOps[Index].HashFinal (HashCtx[0], &Digest);
    }
  }

  return EFI_SUCCESS;
}

/**
  This service registers Hash Interface.

  @param HashInterface  Hash interface

  @retval EFI_SUCCESS          This hash interface is registered successfully.
  @retval EFI_UNSUPPORTED      System does not support register this interface.
  @retval EFI_ALREADY_STARTED  System already register this interface.
**/
EFI_STATUS
EFIAPI
RegisterHashApiLib (
  IN HASH_INTERFACE_UNIFIED_API   *HashInterface
  )
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINT32      HashMask;

  //
  // Check Allow
  //
  HashMask = GetApiHashMaskFromAlgo (&HashInterface->HashGuid);

  // check if Hash Mask is supported
  if ((HashMask & PcdGet32 (PcdTpm2HashMask)) == 0) {
    return EFI_UNSUPPORTED;
  }

  if (mHashInterfaceCount >= sizeof(mHashOps)/sizeof(mHashOps[0])) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Check duplication
  //
  for (Index = 0; Index < mHashInterfaceCount; Index++) {
    if (CompareGuid (&mHashOps[Index].HashGuid, &HashInterface->HashGuid)) {
      DEBUG ((DEBUG_ERROR, "Hash Interface (%g) has been registered\n", &HashInterface->HashGuid));
      return EFI_ALREADY_STARTED;
    }
  }

  //
  // Register the Hash Algo.
  //
  mCurrentHashMask = PcdGet32 (PcdHashAlgorithmBitmap) | HashMask;
  Status = PcdSet32S (PcdHashAlgorithmBitmap, mCurrentHashMask);
  ASSERT_EFI_ERROR (Status);

  CopyMem (&mHashOps[mHashInterfaceCount], HashInterface, sizeof(*HashInterface));
  mHashInterfaceCount ++;

  return EFI_SUCCESS;
}