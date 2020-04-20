/** @file
  Detection code for QEMU debug port.

  Copyright (c) 2017, Red Hat, Inc.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include "DebugLibDetect.h"

/**
  This constructor function must not do anything.

  Some modules consuming this library instance, such as the DXE Core, invoke
  the DEBUG() macro before they explicitly call
  ProcessLibraryConstructorList(). Therefore the auto-generated call from
  ProcessLibraryConstructorList() to this constructor function may be preceded
  by some calls to PlatformDebugLibIoPortFound() below. Hence
  PlatformDebugLibIoPortFound() must not rely on anything this constructor
  could set up.

  @retval RETURN_SUCCESS   The constructor always returns RETURN_SUCCESS.

**/
RETURN_STATUS
EFIAPI
PlatformDebugLibIoPortConstructor (
  VOID
  )
{
  return RETURN_SUCCESS;
}

/**
  Return the result of detecting the debug I/O port device.

  @retval TRUE   if the debug I/O port device was detected.
  @retval FALSE  otherwise

**/
BOOLEAN
EFIAPI
PlatformDebugLibIoPortFound (
  VOID
  )
{
  return TRUE;
}
