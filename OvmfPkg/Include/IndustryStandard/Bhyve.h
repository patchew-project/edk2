/** @file
  Various register numbers and value bits based on FreeBSD's bhyve
  at r359530.
  - https://svnweb.freebsd.org/base?view=revision&revision=359530

  Copyright (C) 2020, Rebecca Cran <rebecca@bsdio.com>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __BHYVE_H__
#define __BHYVE_H__

#include <Library/PciLib.h>
#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>
#include <Protocol/PciRootBridgeIo.h>

#define BHYVE_ACPI_TIMER_IO_ADDR 0x408

#endif // __BHYVE_H__
