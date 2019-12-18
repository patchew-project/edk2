/** @file
  The internal header file includes the common header files, defines
  internal structure and functions used by ImageVerificationLib.

Copyright (c) 2009 - 2019, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BASEHASHLIB_H_
#define __BASEHASHLIB_H_

#include <Uefi.h>
#include <Protocol/Hash.h>
#include <Library/HashLib.h>

//
// Hash Algorithms
//
#define HASH_ALG_DEFAULT    0x00000000
#define HASH_ALG_SHA1       0x00000001
#define HASH_ALG_SHA256     0x00000002
#define HASH_ALG_SHA384     0x00000004
#define HASH_ALG_SHA512     0x00000008
#define HASH_ALG_SM3_256    0x00000010


/**
  Init hash sequence.

  @param HashType   Type of hash algorithm.
  @param HashHandle Hash handle.

  @retval EFI_SUCCESS          Hash start and HashHandle returned.
  @retval EFI_UNSUPPORTED      System has no HASH library registered.
**/
EFI_STATUS
EFIAPI
HashApiInit (
  IN   UINT32         HashType,
  OUT  HASH_HANDLE   *HashHandle
);

/**
  Update hash data.

  @param HashHandle    Hash handle.
  @param HashType   Type of hash algorithm.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.

  @retval EFI_SUCCESS          Hash updated.
  @retval EFI_UNSUPPORTED      System has no HASH library registered.
**/
EFI_STATUS
EFIAPI
HashApiUpdate (
  IN HASH_HANDLE    HashHandle,
  IN UINT32         HashType,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
);

/**
  Hash complete.

  @param HashHandle    Hash handle.
  @param HashType      Type of hash algorithm.
  @param Digest        Hash Digest.

  @retval EFI_SUCCESS     Hash complete and Digest is returned.
**/
EFI_STATUS
EFIAPI
HashApiFinal (
  IN  HASH_HANDLE HashHandle,
  IN  UINT32      HashType,
  OUT UINT8       *Digest
);

/**
  Start hash sequence.

  @param HashHandle Hash handle.

  @retval EFI_SUCCESS          Hash sequence start and HandleHandle returned.
  @retval EFI_OUT_OF_RESOURCES No enough resource to start hash.
**/
typedef
EFI_STATUS
(EFIAPI *BASE_HASH_INIT) (
  OUT HASH_HANDLE    *HashHandle
  );

/**
  Update hash sequence data.

  @param HashHandle    Hash handle.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.

  @retval EFI_SUCCESS     Hash sequence updated.
**/
typedef
EFI_STATUS
(EFIAPI *BASE_HASH_UPDATE) (
  IN HASH_HANDLE    HashHandle,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
  );

/**
  Hash complete.
  @param HashHandle    Hash handle.
  @param Digest        Hash Digest.
  @retval EFI_SUCCESS     Hash complete and Digest is returned.
**/
typedef
EFI_STATUS
(EFIAPI *BASE_HASH_FINAL_EX) (
  IN  HASH_HANDLE      HashHandle,
  OUT UINT8            **Digest
  );

typedef struct {
  EFI_GUID                                HashGuid;
  BASE_HASH_INIT                          HashInit;
  BASE_HASH_UPDATE                        HashUpdate;
  BASE_HASH_FINAL_EX                      HashFinal;
} HASH_INTERFACE_UNIFIED_API;

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
  IN HASH_INTERFACE_UNIFIED_API *HashInterface
);

#endif