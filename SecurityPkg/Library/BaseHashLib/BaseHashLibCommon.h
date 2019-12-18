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

#ifndef __BASEHASHLIB_COMMON_H_
#define __BASEHASHLIB_COMMON_H_

#define HASH_ALGO_COUNT 5


typedef struct {
  EFI_GUID  Guid;
  UINT32    Mask;
} HASH_MASK;

HASH_MASK mHashMask[] = {
  {HASH_ALGORITHM_SHA1_GUID,         HASH_ALG_SHA1},
  {HASH_ALGORITHM_SHA256_GUID,       HASH_ALG_SHA256},
  {HASH_ALGORITHM_SHA384_GUID,       HASH_ALG_SHA384},
  {HASH_ALGORITHM_SHA512_GUID,       HASH_ALG_SHA512},
  {HASH_ALGORITHM_SM3_256_GUID,      HASH_ALG_SM3_256},
};

#endif