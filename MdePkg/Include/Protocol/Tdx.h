/** @file
  Tcg for Intel TDX definitions.

Copyright (c) 2020 - 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#ifndef __TCG_TDX_H__
#define __TCG_TDX_H__

#include <Uefi/UefiBaseType.h>

#define TCG_TDX_EVENT_DATA_SIGNATURE  SIGNATURE_32 ('T', 'D', 'X', 'S')

#define TD_TCG2_PROTOCOL_GUID  \
  {0x96751a3d, 0x72f4, 0x41a6, { 0xa7, 0x94, 0xed, 0x5d, 0x0e, 0x67, 0xae, 0x6b }}
extern EFI_GUID gTdTcg2ProtocolGuid;


#endif
