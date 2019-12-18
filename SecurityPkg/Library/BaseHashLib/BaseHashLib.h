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

#define HASH_ALGO_COUNT 7

//
// Hash Algorithms
//
#define HASH_ALG_SHA1    0x00000001
#define HASH_ALG_SHA256  0x00000002
#define HASH_ALG_SHA384  0x00000004
#define HASH_ALG_SHA512  0x00000008
#define HASH_ALG_SM3_256 0x00000010
#if 0
typedef 
UINTN
(EFIAPI *GET_HASH_CTX_SIZE) (
  VOID
  );

typedef
BOOLEAN
(EFIAPI *_HASH_INIT) (
  OUT  VOID  *ShaContext
  );

typedef
BOOLEAN
(EFIAPI *_HASH_DUPLICATE) (
  IN   CONST VOID  *ShaContext,
  OUT  VOID        *NewShaContext
  );

typedef
BOOLEAN
(EFIAPI *_HASH_UPDATE) (
  IN OUT  VOID        *ShaContext,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  );

typedef
BOOLEAN
(EFIAPI *_HASH_FINAL) (
  IN OUT  VOID   *ShaContext,
  OUT     UINT8  *HashValue
  );

HASH_ALGO_IDX
GetHashAlgoIndex (
  VOID
);

typedef struct {
  HASH_ALGO_IDX     HashAlgo;
  GET_HASH_CTX_SIZE  GetHashCtxSize;
  _HASH_INIT          HashInit;
  _HASH_DUPLICATE     HashDuplicate;
  _HASH_UPDATE        HashUpdate;
  _HASH_FINAL         HashFinal;
} HASH_OPERATIONS;


EFI_STATUS
EFIAPI
RegisterHashLib (
  IN HASH_OPERATIONS   *HashInterface
);
#endif
#endif