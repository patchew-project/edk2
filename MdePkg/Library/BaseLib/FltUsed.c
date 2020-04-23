/** @file
  Declare _fltused symbol for MSVC

  MSVC need this symbol for float, andd it here to feed MSVC. it may remove
  if MSVC not need it any more.

  Copyright (c) 2020 - 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

//
// Just for MSVC float
//
int _fltused = 0x1;
