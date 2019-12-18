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
#include <Library/BaseHashLib.h>

#include "BaseHashLibCommon.h"


HASH_INTERFACE_UNIFIED_API mHashOps[HASH_ALGO_COUNT] = {{{0}, NULL, NULL, NULL}};

UINTN mBaseHashInterfaceCount = 0;
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
  UINT32  HashPolicy;

  HashPolicy = PcdGet32 (PcdSystemHashPolicy);

  if ((mBaseHashInterfaceCount == 0) || !(mCurrentHashMask & HashPolicy)) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = AllocatePool (sizeof(*HashCtx));
  ASSERT (HashCtx != NULL);

  for (Index = 0; Index < mBaseHashInterfaceCount; Index++) {
    HashMask = GetApiHashMaskFromAlgo (&mHashOps[Index].HashGuid);
    if ((HashMask & HashPolicy) != 0) {
      mHashOps[Index].HashInit (HashCtx);
      break;
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
  UINT32       HashPolicy;

  HashPolicy = PcdGet32 (PcdSystemHashPolicy);

  if ((mBaseHashInterfaceCount == 0) || !(mCurrentHashMask & HashPolicy)) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = (HASH_HANDLE *)HashHandle;

  for (Index = 0; Index < mBaseHashInterfaceCount; Index++) {
    HashMask = GetApiHashMaskFromAlgo (&mHashOps[Index].HashGuid);
    if ((HashMask & HashPolicy) != 0) {
      mHashOps[Index].HashUpdate (HashCtx[0], DataToHash, DataToHashLen);
      break;
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
  UINT32       HashPolicy;

  HashPolicy = PcdGet32 (PcdSystemHashPolicy);

  if ((mBaseHashInterfaceCount == 0) || !(mCurrentHashMask & HashPolicy)) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = (HASH_HANDLE *)HashHandle;

  for (Index = 0; Index < mBaseHashInterfaceCount; Index++) {
    HashMask = GetApiHashMaskFromAlgo (&mHashOps[Index].HashGuid);
    if ((HashMask & HashPolicy) != 0) {
      mHashOps[Index].HashFinal (HashCtx[0], &Digest);
      break;
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
  UINTN       Index;
  UINT32      HashMask;

  //
  // Check Allow
  //
  HashMask = GetApiHashMaskFromAlgo (&HashInterface->HashGuid);


  if (mBaseHashInterfaceCount >= sizeof(mHashOps)/sizeof(mHashOps[0])) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Check duplication
  //
  for (Index = 0; Index < mBaseHashInterfaceCount; Index++) {
    if (CompareGuid (&mHashOps[Index].HashGuid, &HashInterface->HashGuid)) {
      DEBUG ((DEBUG_ERROR, "Hash Interface (%g) has already been registered\n", &HashInterface->HashGuid));
      return EFI_ALREADY_STARTED;
    }
  }

  //
  // Register the Hash Algo.
  //
  mCurrentHashMask = mCurrentHashMask | HashMask;

  CopyMem (&mHashOps[mBaseHashInterfaceCount], HashInterface, sizeof(*HashInterface));
  mBaseHashInterfaceCount ++;

  DEBUG ((DEBUG_INFO,"RegisterHashApiLib: mBaseHashInterfaceCount update to 0x%x \n", mBaseHashInterfaceCount));

  return EFI_SUCCESS;
}

/**
  The constructor function of BaseHashLib Dxe.

  @param  FileHandle   The handle of FFS header the loaded driver.
  @param  PeiServices  The pointer to the PEI services.

  @retval EFI_SUCCESS           The constructor executes successfully.
  @retval EFI_OUT_OF_RESOURCES  There is no enough resource for the constructor.

**/
EFI_STATUS
EFIAPI
BaseHashLibApiDxeConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  mBaseHashInterfaceCount = 0;
  mCurrentHashMask = 0;

  return EFI_SUCCESS;
}