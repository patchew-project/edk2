/** @file
  PCI memory configuration for Synquacer

  Copyright (c) 2017, Linaro Ltd. All rights reserved.<BR>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _SYNQUACER_PLATFORM_PCI_H_
#define _SYNQUACER_PLATFORM_PCI_H_

#define SYNQUACER_PCI_SEG0_CONFIG_BASE      0x60000000
#define SYNQUACER_PCI_SEG0_CONFIG_SIZE      0x07f00000
#define SYNQUACER_PCI_SEG0_DBI_BASE         0x583d0000
#define SYNQUACER_PCI_SEG0_EXS_BASE         0x58390000

#define SYNQUACER_PCI_SEG0_BUSNUM_MIN       0x0
#define SYNQUACER_PCI_SEG0_BUSNUM_MAX       0x7e

#define SYNQUACER_PCI_SEG0_PORTIO_MIN       0x0
#define SYNQUACER_PCI_SEG0_PORTIO_MAX       0xffff
#define SYNQUACER_PCI_SEG0_PORTIO_SIZE      0x10000
#define SYNQUACER_PCI_SEG0_PORTIO_MEMBASE   0x67f00000
#define SYNQUACER_PCI_SEG0_PORTIO_MEMSIZE   SYNQUACER_PCI_SEG0_PORTIO_SIZE

#define SYNQUACER_PCI_SEG0_MMIO32_MIN       0x68000000
#define SYNQUACER_PCI_SEG0_MMIO32_MAX       0x6fffffff
#define SYNQUACER_PCI_SEG0_MMIO32_SIZE      0x08000000

#define SYNQUACER_PCI_SEG0_MMIO64_MIN       0x3e00000000
#define SYNQUACER_PCI_SEG0_MMIO64_MAX       0x3effffffff
#define SYNQUACER_PCI_SEG0_MMIO64_SIZE      0x100000000

#define SYNQUACER_PCI_SEG1_CONFIG_BASE      0x70000000
#define SYNQUACER_PCI_SEG1_CONFIG_SIZE      0x07f00000
#define SYNQUACER_PCI_SEG1_DBI_BASE         0x583c0000
#define SYNQUACER_PCI_SEG1_EXS_BASE         0x58380000

#define SYNQUACER_PCI_SEG1_BUSNUM_MIN       0x0
#define SYNQUACER_PCI_SEG1_BUSNUM_MAX       0x7e

#define SYNQUACER_PCI_SEG1_PORTIO_MIN       0x10000
#define SYNQUACER_PCI_SEG1_PORTIO_MAX       0x1ffff
#define SYNQUACER_PCI_SEG1_PORTIO_SIZE      0x10000
#define SYNQUACER_PCI_SEG1_PORTIO_MEMBASE   0x77f00000
#define SYNQUACER_PCI_SEG1_PORTIO_MEMSIZE   SYNQUACER_PCI_SEG1_PORTIO_SIZE

#define SYNQUACER_PCI_SEG1_MMIO32_MIN       0x78000000
#define SYNQUACER_PCI_SEG1_MMIO32_MAX       0x7fffffff
#define SYNQUACER_PCI_SEG1_MMIO32_SIZE      0x08000000

#define SYNQUACER_PCI_SEG1_MMIO64_MIN       0x3f00000000
#define SYNQUACER_PCI_SEG1_MMIO64_MAX       0x3fffffffff
#define SYNQUACER_PCI_SEG1_MMIO64_SIZE      0x100000000

#endif
