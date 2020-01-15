/** @file
  The internal header file includes the common header files, defines
  internal structure and functions used by ImageVerificationLib.

Copyright (c) 2009 - 2020, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BASEHASHLIB_COMMON_H_
#define __BASEHASHLIB_COMMON_H_

/**
  Init hash sequence with Hash Algorithm specified by HashPolicy.

  @param HashHandle Hash handle.

  @retval EFI_SUCCESS          Hash start and HashHandle returned.
  @retval EFI_UNSUPPORTED      System has no HASH library registered.
**/
BOOLEAN
EFIAPI
HashInitInternal (
  IN UINT8          HashPolicy,
  OUT HASH_HANDLE   *HashHandle
  );

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
HashUpdateInternal (
  IN UINT8        HashPolicy,
  IN HASH_HANDLE  HashHandle,
  IN VOID         *DataToHash,
  IN UINTN        DataToHashLen
  );

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
HashFinalInternal (
  IN UINT8        HashPolicy,
  IN HASH_HANDLE  HashHandle,
  OUT UINT8       **Digest
  );

/**
  Retrieves the size, in bytes, of the context buffer required for hash operations.

  @return  The size, in bytes, of the context buffer required for hash operations.

**/
typedef
UINTN
(EFIAPI *HASH_API_GET_CONTEXT_SIZE) (
  VOID
  );

/**
  Start hash.

  @param HashCtx    Hash Context.

  @retval EFI_SUCCESS          Hash start and HashHandle returned.
  @retval EFI_UNSUPPORTED      Unsupported Hash Policy specified.
**/
typedef
BOOLEAN
(EFIAPI *HASH_API_INIT) (
  OUT     VOID        *HashCtx
  );


/**
  Update hash data.

  @param HashCtx    Hash Context.
  @param Data       Data to be hashed.
  @param DataSize   Data size.

  @retval TRUE         Hash updated.
  @retval FALSE        Hash updated unsuccessful or hash unsupported.
**/
typedef
BOOLEAN
(EFIAPI *HASH_API_UPDATE) (
  IN OUT  VOID        *HashCtx,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  );

/**
  Hash complete.

  @param HashCtx  Hash Context.
  @param Digest   Digest.

  @retval TRUE         Hash complete and Digest is returned.
  @retval FALSE        Hash complete unsuccessful.
**/
typedef
BOOLEAN
(EFIAPI *HASH_API_FINAL) (
  IN OUT  VOID        *HashCtx,
  OUT     UINT8       *Digest
  );

typedef struct {
  HASH_API_GET_CONTEXT_SIZE   HashGetContextSize;
  HASH_API_INIT               HashInit;
  HASH_API_UPDATE             HashUpdate;
  HASH_API_FINAL              HashFinal;
  UINTN                       DigestSize;
} HASH_API_INTERFACE;

#endif
