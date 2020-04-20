/** @file
  Base Debug library instance for QEMU debug port.
  It uses PrintLib to send debug messages to a fixed I/O port.

  Copyright (c) 2017, Red Hat, Inc.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __DEBUG_IO_PORT_DETECT_H__
#define __DEBUG_IO_PORT_DETECT_H__

#include <Base.h>


/**
  Return whether the virtual machine has a debug I/O port.  DebugLib.c
  calls this function instead of PlatformDebugLibIoPortDetect, to allow
  caching if possible.

  @retval TRUE   if the debug I/O port device was detected.
  @retval FALSE  otherwise

**/
BOOLEAN
EFIAPI
PlatformDebugLibIoPortFound (
  VOID
  );

#endif
