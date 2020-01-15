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

Copyright (c) 2009 - 2020, Intel Corporation. All rights reserved.<BR>
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
#include <Library/BaseCryptLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseHashLib.h>

#include "BaseHashLibCommon.h"

STATIC CONST HASH_API_INTERFACE mHashApi [] = {
  {NULL, NULL, NULL, NULL},
  {Md4GetContextSize,     Md4Init,    Md4Update,    Md4Final,     MD4_DIGEST_SIZE},
  {Md5GetContextSize,     Md5Init,    Md5Update,    Md5Final,     MD5_DIGEST_SIZE},
  {Sha1GetContextSize,    Sha1Init,   Sha1Update,   Sha1Final,    SHA1_DIGEST_SIZE},
  {Sha256GetContextSize,  Sha256Init, Sha256Update, Sha256Final,  SHA256_DIGEST_SIZE},
  {Sha384GetContextSize,  Sha384Init, Sha384Update, Sha384Final,  SHA384_DIGEST_SIZE},
  {Sha512GetContextSize,  Sha512Init, Sha512Update, Sha512Final,  SHA512_DIGEST_SIZE},
  {Sm3GetContextSize,     Sm3Init,    Sm3Update,    Sm3Final,     SM3_256_DIGEST_SIZE}
};

/**
  Init hash sequence with Hash Algorithm specified by HashPolicy.

  @param HashPolicy  Hash Algorithm Policy.
  @param HashHandle  Hash handle.

  @retval TRUE       Hash start and HashHandle returned.
  @retval FALSE      Hash Init unsuccessful.
**/
BOOLEAN
EFIAPI
HashInitInternal (
  IN UINT8          HashPolicy,
  OUT HASH_HANDLE   *HashHandle
  )
{
  BOOLEAN  Status;
  VOID     *HashCtx;
  UINTN    CtxSize;

  if (HashPolicy == HASH_INVALID || HashPolicy >= HASH_MAX) {
    ASSERT (FALSE);
  }

  CtxSize = mHashApi[HashPolicy].HashGetContextSize ();
  HashCtx = AllocatePool (CtxSize);
  ASSERT (HashCtx != NULL);

  Status = mHashApi[HashPolicy].HashInit (HashCtx);

  *HashHandle = (HASH_HANDLE)HashCtx;

  return Status;
}

/**
  Update hash data with Hash Algorithm specified by HashPolicy.

  @param HashPolicy    Hash Algorithm Policy.
  @param HashHandle    Hash handle.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.

  @retval TRUE         Hash updated.
  @retval FALSE        Hash updated unsuccessful.
**/
BOOLEAN
EFIAPI
HashUpdateInternal (
  IN UINT8        HashPolicy,
  IN HASH_HANDLE  HashHandle,
  IN VOID         *DataToHash,
  IN UINTN        DataToHashLen
  )
{
  BOOLEAN  Status;
  VOID     *HashCtx;

  if (HashPolicy == HASH_INVALID || HashPolicy >= HASH_MAX) {
    ASSERT (FALSE);
  }

  HashCtx = (VOID *)HashHandle;

  Status = mHashApi[HashPolicy].HashUpdate (HashCtx, DataToHash, DataToHashLen);

  return Status;
}

/**
  Hash complete with Hash Algorithm specified by HashPolicy.

  @param HashPolicy    Hash Algorithm Policy.
  @param HashHandle    Hash handle.
  @param Digest        Hash Digest.

  @retval TRUE         Hash complete and Digest is returned.
  @retval FALSE        Hash complete unsuccessful.
**/
BOOLEAN
EFIAPI
HashFinalInternal (
  IN UINT8        HashPolicy,
  IN HASH_HANDLE  HashHandle,
  OUT UINT8       **Digest
  )
{
  BOOLEAN  Status;
  VOID     *HashCtx;
  UINT8    DigestData[SHA512_DIGEST_SIZE];

  if (HashPolicy == HASH_INVALID || HashPolicy >= HASH_MAX) {
    ASSERT (FALSE);
  }

  HashCtx = (VOID *)HashHandle;

  Status = mHashApi[HashPolicy].HashFinal (HashCtx, DigestData);
  CopyMem (*Digest, DigestData, mHashApi[HashPolicy].DigestSize);

  FreePool (HashCtx);

  return Status;
}
