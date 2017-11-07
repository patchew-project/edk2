/** Utils.c

  Copyright 2017 NXP

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/Utils.h>

/* Function to convert a frequency to MHz */
CHAR8 *
StringToMHz (
  IN  CHAR8   *Buf,
  IN  UINT32  Size,
  IN  UINT64  Hz
  )
{
  UINT64 L;
  UINT64 M;
  UINT64 N;

  N = DIV_ROUND_CLOSEST(Hz, 1000) / 1000L;
  L = AsciiSPrint (Buf, Size, "%ld", N);

  Hz -= N * 1000000L;
  M = DIV_ROUND_CLOSEST(Hz, 1000L);

  if (M != 0) {
    AsciiSPrint (Buf + L, Size, ".%03ld", M);
  }

  return (Buf);
}

/*
 * PrINT32 Sizes As "Xxx KiB", "Xxx.Y KiB", "Xxx MiB", "Xxx.Y MiB",
 * Xxx GiB, Xxx.Y GiB, Etc As Needed; Allow for Optional Trailing String
 * (Like "\n")
 */
VOID
PrintSize (
  IN  UINT64 Size,
  IN  CONST  INT8 *S
  )
{
  UINT64 M;
  UINT64 N;
  UINT64 F;
  UINT64 D;
  CHAR8 C;
  UINT32 I;
  INT8 Names[6] = {'E', 'P', 'T', 'G', 'M', 'K'};

  M = 0;
  D = 10 * ARRAY_SIZE(Names);
  C = 0;

  for (I = 0; I < ARRAY_SIZE(Names); I++, D -= 10) {
    if (Size >> D) {
      C = Names[I];
      break;
    }
  }

  if (!C) {
    DEBUG((DEBUG_ERROR, "%Ld Bytes,\n %a", Size, S));
    return;
  }

  N = Size >> D;
  F = Size & ((1ULL << D) - 1);

  /* if There'S A Remainder, Deal With It */
  if (F) {
    M = (10ULL * F + (1ULL << (D - 1))) >> D;

    if (M >= 10) {
           M -= 10;
           N += 1;
    }
  }

  DEBUG((DEBUG_ERROR, "%Ld", N));
  if (M) {
    DEBUG((DEBUG_ERROR, ".%Ld", M));
  }
  DEBUG((DEBUG_ERROR, " %ciB, %a ", C, S));
}
