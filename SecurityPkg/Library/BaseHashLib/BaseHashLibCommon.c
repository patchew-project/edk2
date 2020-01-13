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

  switch (HashPolicy) {
    case HASH_MD4:
      CtxSize = Md4GetContextSize ();
      HashCtx = AllocatePool (CtxSize);
      ASSERT (HashCtx != NULL);

      Status = Md4Init (HashCtx);
      break;

    case HASH_MD5:
      CtxSize = Md5GetContextSize ();
      HashCtx = AllocatePool (CtxSize);
      ASSERT (HashCtx != NULL);

     Status = Md5Init (HashCtx);
      break;

    case HASH_SHA1:
      CtxSize = Sha1GetContextSize ();
      HashCtx = AllocatePool (CtxSize);
      ASSERT (HashCtx != NULL);

      Status = Sha1Init (HashCtx);
      break;
    
    case HASH_SHA256:
      CtxSize = Sha256GetContextSize ();
      HashCtx = AllocatePool (CtxSize);
      ASSERT (HashCtx != NULL);

      Status = Sha256Init (HashCtx);
      break;

    case HASH_SHA384:
      CtxSize = Sha384GetContextSize ();
      HashCtx = AllocatePool (CtxSize);
      ASSERT (HashCtx != NULL);

      Status = Sha384Init (HashCtx);
      break;

    case HASH_SHA512:
      CtxSize = Sha512GetContextSize ();
      HashCtx = AllocatePool (CtxSize);
      ASSERT (HashCtx != NULL);

      Status = Sha512Init (HashCtx);
      break;

    case HASH_SM3_256:
      CtxSize = Sm3GetContextSize ();
      HashCtx = AllocatePool (CtxSize);
      ASSERT (HashCtx != NULL);

      Status = Sm3Init (HashCtx);
      break;

    default:
      ASSERT (FALSE);
      break;
  }

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

  HashCtx = (VOID *)HashHandle;

  switch (HashPolicy) {
    case HASH_MD4:
      Status = Md4Update (HashCtx, DataToHash, DataToHashLen);
      break;

    case HASH_MD5:
      Status = Md5Update (HashCtx, DataToHash, DataToHashLen);
      break;

    case HASH_SHA1:
      Status = Sha1Update (HashCtx, DataToHash, DataToHashLen);
      break;
    
    case HASH_SHA256:
      Status = Sha256Update (HashCtx, DataToHash, DataToHashLen);
      break;

    case HASH_SHA384:
      Status = Sha384Update (HashCtx, DataToHash, DataToHashLen);
      break;

    case HASH_SHA512:
      Status = Sha512Update (HashCtx, DataToHash, DataToHashLen);
      break;

    case HASH_SM3_256:
      Status = Sm3Update (HashCtx, DataToHash, DataToHashLen);
      break;

    default:
      ASSERT (FALSE);
      break;
  }

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

  HashCtx = (VOID *)HashHandle;

  switch (HashPolicy) {
    case HASH_MD4:
      Status = Md4Final (HashCtx, DigestData);
      CopyMem (*Digest, DigestData, MD4_DIGEST_SIZE);
      break;

    case HASH_MD5:
      Status = Md5Final (HashCtx, DigestData);
      CopyMem (*Digest, DigestData, MD5_DIGEST_SIZE);
      break;

    case HASH_SHA1:
      Status = Sha1Final (HashCtx, DigestData);
      CopyMem (*Digest, DigestData, SHA1_DIGEST_SIZE);
      break;
    
    case HASH_SHA256:
      Status = Sha256Final (HashCtx, DigestData);
      CopyMem (*Digest, DigestData, SHA256_DIGEST_SIZE);
      break;

    case HASH_SHA384:
      Status = Sha384Final (HashCtx, DigestData);
      CopyMem (*Digest, DigestData, SHA384_DIGEST_SIZE);
      break;

    case HASH_SHA512:
      Status = Sha512Final (HashCtx, DigestData);
      CopyMem (*Digest, DigestData, SHA512_DIGEST_SIZE);
      break;

    case HASH_SM3_256:
      Status = Sm3Final (HashCtx, DigestData);
      CopyMem (*Digest, DigestData, SM3_256_DIGEST_SIZE);
      break;

    default:
      ASSERT (FALSE);
      break;
  }

  FreePool (HashCtx);

  return Status;
}