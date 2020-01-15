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

#ifndef __BASEHASHLIB_H_
#define __BASEHASHLIB_H_

#include <Uefi.h>
#include <Protocol/Hash.h>
#include <Library/HashLib.h>

//
// Hash Algorithms
//
#define HASH_INVALID    0x00000000
#define HASH_MD4        0x00000001
#define HASH_MD5        0x00000002
#define HASH_SHA1       0x00000003
#define HASH_SHA256     0x00000004
#define HASH_SHA384     0x00000005
#define HASH_SHA512     0x00000006
#define HASH_SM3_256    0x00000007
#define HASH_MAX        0x00000008


/**
  Init hash sequence.

  @param HashHandle  Hash handle.

  @retval TRUE       Hash start and HashHandle returned.
  @retval FALSE      Hash Init unsuccessful.
**/
BOOLEAN
EFIAPI
HashApiInit (
  OUT  HASH_HANDLE   *HashHandle
);

/**
  Update hash data.

  @param HashHandle    Hash handle.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.

  @retval TRUE         Hash updated.
  @retval FALSE        Hash updated unsuccessful.
**/
BOOLEAN
EFIAPI
HashApiUpdate (
  IN HASH_HANDLE    HashHandle,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
);

/**
  Hash complete.

  @param HashHandle    Hash handle.
  @param Digest        Hash Digest.

  @retval TRUE         Hash complete and Digest is returned.
  @retval FALSE        Hash complete unsuccessful.
**/
BOOLEAN
EFIAPI
HashApiFinal (
  IN  HASH_HANDLE HashHandle,
  OUT UINT8       *Digest
);

#endif
