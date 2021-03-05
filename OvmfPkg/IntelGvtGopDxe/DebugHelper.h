/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __DEBUGHELPER_H_
#define __DEBUGHELPER_H_

#include <Library/DebugLib.h>

#define GVT_DEBUG(ErrLevel, Fmt, Args...) \
  do { \
    DEBUG ((ErrLevel, "GvtGop: "Fmt, ##Args));\
  } while (FALSE)

#endif //__DEBUGHELPER_H_
