/** @file
  CmosAccessLib internal header file.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under
the terms and conditions of the BSD License that accompanies this distribution.
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _CMOS_ACCESS_LIB_INTERNALS_
#define _CMOS_ACCESS_LIB_INTERNALS_

#include <Base.h>
#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/CmosAccessLib.h>
#include <Library/PlatformCmosAccessLib.h>

// CMOS access Port address

#define PORT_70            0x70
#define PORT_71            0x71
#define PORT_72            0x72
#define PORT_73            0x73

#define CMOS_BANK0_LIMIT   0x7F
#define CMOS_BANK1_LIMIT   0xFF

typedef struct {
   UINT8  Length;
   UINT8  LowByteAddress;
   UINT8  HighByteAddress;
} CMOS_CHECKSUM_LOCATION_INFO;

#endif // _CMOS_ACCESS_LIB_INTERNALS_
