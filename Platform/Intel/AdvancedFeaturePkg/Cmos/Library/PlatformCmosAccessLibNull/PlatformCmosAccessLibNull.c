/** @file
  Platform CMOS Access Library.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under
the terms and conditions of the BSD License that accompanies this distribution.
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library/PlatformCmosAccessLib.h>

/**
  Return the platform CMOS entries.

  @param [out] EntryCount Return the count of platform CMOS entries.

  @return Platform CMOS entries.
**/
CMOS_ENTRY *
EFIAPI
PlatformCmosGetEntry (
  OUT UINTN       *EntryCount
  )
{
  *EntryCount = 0;
  return NULL;
}

/**
  Return the NMI enable status.
**/
BOOLEAN
EFIAPI
PlatformCmosGetNmiState (
  VOID
  )
{
  return FALSE;
}
