/** @file

  Copyright (c) 2011 - 2013, Intel Corporation. All rights reserved.<BR>
  Copyright (C) 2013, Red Hat, Inc.
  Copyright (c) 2015, Nahanni Systems.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Uefi.h"
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BhyveFwCtlLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>


/**
   Front end to the internal GET_LEN and GET protocols
 **/
RETURN_STATUS
EFIAPI
BhyveFwCtlGet (
  IN   CONST CHAR8    *Name,
  OUT  VOID        *Item,
  IN OUT  UINTN        *Size
  )
{
  return RETURN_UNSUPPORTED;
}


/**
   Library initialization. Probe the host to see if the f/w ctl
   interface is supported.
 **/
RETURN_STATUS
EFIAPI
BhyveFwCtlInitialize (
          VOID
         )
{
  return RETURN_SUCCESS;
}
