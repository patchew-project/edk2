/** Utils.h
  Header defining the General Purpose Utilities

  Copyright 2017 NXP

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __UTILS_H__
#define __UTILS_H__

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest UINTNeger. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(X, Divisor)(           \
{                                                \
  typeof(X) __X = X;                             \
  typeof(Divisor) __D = Divisor;                 \
  (((typeof(X))-1) > 0 ||                        \
    ((typeof(Divisor))-1) > 0 || (__X) > 0) ?    \
      (((__X) + ((__D) / 2)) / (__D)) :          \
      (((__X) - ((__D) / 2)) / (__D));           \
}                                                \
)

/*
 * HammingWeight32: returns the hamming weight (i.e. the number
 * of bits set) of a 32-bit word
 */
STATIC
inline
UINTN
HammingWeight32 (
  IN  UINTN  W
  )
{
  UINTN Res;

  Res = (W & 0x55555555) + ((W >> 1) & 0x55555555);
  Res = (Res & 0x33333333) + ((Res >> 2) & 0x33333333);
  Res = (Res & 0x0F0F0F0F) + ((Res >> 4) & 0x0F0F0F0F);
  Res = (Res & 0x00FF00FF) + ((Res >> 8) & 0x00FF00FF);

  return (Res & 0x0000FFFF) + ((Res >> 16) & 0x0000FFFF);
}

STATIC
inline
UINTN
CpuMaskNext (
  IN  UINTN  Cpu,
  IN  UINTN  Mask
  )
{
  for (Cpu++; !((1 << Cpu) & Mask); Cpu++)
    ;

  return Cpu;
}

#define ForEachCpu(Iter, Cpu, NumCpus, Mask) \
  for (Iter = 0, Cpu = CpuMaskNext(-1, Mask); \
    Iter < NumCpus; \
    Iter++, Cpu = CpuMaskNext(Cpu, Mask)) \

/**
  Find last (most-significant) bit set

  @param   X :        the word to search

  Note Fls(0) = 0, Fls(1) = 1, Fls(0x80000000) = 32.

**/
STATIC
inline
INT32
GenericFls (
  IN  INT32  X
  )
{
  INT32 R = 32;

  if (!X)
    return 0;

  if (!(X & 0xffff0000u)) {
    X <<= 16;
    R -= 16;
  }
  if (!(X & 0xff000000u)) {
    X <<= 8;
    R -= 8;
  }
  if (!(X & 0xf0000000u)) {
    X <<= 4;
    R -= 4;
  }
  if (!(X & 0xc0000000u)) {
    X <<= 2;
    R -= 2;
  }
  if (!(X & 0x80000000u)) {
    X <<= 1;
    R -= 1;
  }

  return R;
}

/*
 * PrINT32 Sizes As "Xxx KiB", "Xxx.Y KiB", "Xxx MiB", "Xxx.Y MiB",
 * Xxx GiB, Xxx.Y GiB, Etc As Needed; Allow for Optional Trailing String
 * (Like "\n")
 */
VOID
PrintSize (
  IN  UINT64 Size,
  IN  CONST INT8 *S
  );

/* Function to convert a frequency to MHz */
CHAR8 *StringToMHz (
  CHAR8   *Buf,
  UINT32  Size,
  UINT64  Hz
  );

#endif
