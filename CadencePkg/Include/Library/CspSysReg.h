/** @file
*  Cadence CSP system register offsets.
*  Copyright (c) 2017, Cadence Design Systems. All rights reserved.
*
*  This program and the accompanying materials are licensed and made
*  available under the terms and conditions of the BSD License which
*  accompanies this distribution.  The full text of the license may be
*  found at http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#ifndef __CSP_SYS_REG_H__
#define __CSP_SYS_REG_H__

// Cadence CSP system register offsets
#define CSP_SYSREG_VERSION          0x00
#define CSP_SYSREG_CPU_FREQ         0x04
#define CSP_SYSREG_STATUS           0x08
#define CSP_SYSREG_RUN_STALL        0x0C
#define CSP_SYSREG_SW_RESET         0x10
#define CSP_SYSREG_CORE1_RESET      0x14
#define CSP_SYSREG_SCRATCH_REG_0    0x18
#define CSP_SYSREG_PROC_INTERRUPT   0x1C
#define CSP_SYSREG_SCRATCH_REG_1    0x20
#define CSP_SYSREG_SCRATCH_REG_2    0x24
#define CSP_SYSREG_SCRATCH_REG_3    0x28
#define CSP_SYSREG_SCRATCH_REG_4    0x2C
#define CSP_SYSREG_SCRATCH_REG_5    0x30
#define CSP_SYSREG_SCRATCH_REG_6    0x34
#define CSP_SYSREG_SET_INTERRUPT    0x38
#define CSP_SYSREG_CLR_INTERRUPT    0x3C
#define CSP_SYSREG_DIP_SWITCHES     0x40

#endif
