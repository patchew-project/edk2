/** @file
*
* Copyright (c) 2018 - 2020, ARM Limited. All rights reserved.
*
* SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#ifndef __N1SDP_PLATFORM_H__
#define __N1SDP_PLATFORM_H__

#define N1SDP_DRAM_BLOCK1_SIZE               SIZE_2GB

//******************************************************************************
// Platform Memory Map
//******************************************************************************

// SubSystem Peripherals - UART0
#define N1SDP_UART0_BASE                     0x2A400000
#define N1SDP_UART0_SZ                       SIZE_64KB

// SubSystem Peripherals - UART1
#define N1SDP_UART1_BASE                     0x2A410000
#define N1SDP_UART1_SZ                       SIZE_64KB

// SubSystem Peripherals - Generic Watchdog
#define N1SDP_GENERIC_WDOG_BASE              0x2A440000
#define N1SDP_GENERIC_WDOG_SZ                SIZE_128KB

// SubSystem Peripherals - GIC(600)
#define N1SDP_GIC_BASE                       0x30000000
#define N1SDP_GICR_BASE                      0x300C0000
#define N1SDP_GIC_SZ                         SIZE_256KB
#define N1SDP_GICR_SZ                        SIZE_1MB

// SubSystem non-secure SRAM
#define N1SDP_NON_SECURE_SRAM_BASE           0x06000000
#define N1SDP_NON_SECURE_SRAM_SZ             SIZE_64KB

// AXI Expansion peripherals
#define N1SDP_EXP_PERIPH_BASE0               0x1C000000
#define N1SDP_EXP_PERIPH_BASE0_SZ            0x1300000

// Base address to a structure of type N1SDP_PLAT_INFO which is pre-populated
// by a earlier boot stage
#define N1SDP_PLAT_INFO_STRUCT_BASE          (N1SDP_NON_SECURE_SRAM_BASE + \
                                              0x00008000)

/*
 * Platform information structure stored in non secure SRAM
 * Platform information are passed from the trusted firmware with the below
 * structure format. The elements of N1SDP_PLAT_INFO should be always in sync
 * with the structure in trusted firmware
 */
#pragma pack(1)
typedef struct {
  /*! 0 - Single Chip, 1 - Chip to Chip (C2C) */
  BOOLEAN MultichipMode;
  /*! Slave count in C2C mode */
  UINT8   SlaveCount;
  /*! Local DDR memory size in GigaBytes */
  UINT8   LocalDdrSize;
  /*! Remote DDR memory size in GigaBytes */
  UINT8   RemoteDdrSize;
} N1SDP_PLAT_INFO;
#pragma pack()

#endif
