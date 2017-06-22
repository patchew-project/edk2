/** @file
*  Serial Port Library for Cadence IP6528 UART.
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

#ifndef __CSP_SERIAL_PORT_LIB_H__
#define __CSP_SERIAL_PORT_LIB_H__

// Cadence UART register offsets
#define CSP_UART_CR             0x00  // Control
#define CSP_UART_MR             0x04  // Mode
#define CSP_UART_IER            0x08  // Interrupt enable
#define CSP_UART_IDR            0x0C  // Interrupt disable
#define CSP_UART_IMR            0x10  // Interrupt mask
#define CSP_UART_CISR           0x14  // Channel interrupt status
#define CSP_UART_BRGR           0x18  // Baud rate generator
#define CSP_UART_RTOR           0x1C  // Rx Timeout
#define CSP_UART_RTRIG          0x20  // Rx FIFO trigger level
#define CSP_UART_MCR            0x24  // Modem control
#define CSP_UART_MSR            0x28  // Modem status
#define CSP_UART_CSR            0x2C  // Channel status
#define CSP_UART_FIFO           0x30  // FIFO (Tx/Rx)
#define CSP_UART_BDIV           0x34  // Baud rate divider
#define CSP_UART_FDEL           0x38  // Flow delay
#define CSP_UART_PMIN           0x3C  // IR min received pulse width
#define CSP_UART_PWID           0x40  // IR transmitted pulse Width
#define CSP_UART_TTRIG          0x44  // Tx FIFO trigger level


// Control Register Bit Definitions
#define CSP_UART_CR_STPBRK      0x00000100  // Stop Tx break
#define CSP_UART_CR_STTBRK      0x00000080  // Start Tx break
#define CSP_UART_CR_RSTTO       0x00000040  // Restart Rx timeout Counter
#define CSP_UART_CR_TXDIS       0x00000020  // Tx disable
#define CSP_UART_CR_TXEN        0x00000010  // Tx enable
#define CSP_UART_CR_RXDIS       0x00000008  // Rx disable
#define CSP_UART_CR_RXEN        0x00000004  // Rx enable
#define CSP_UART_CR_TXRES       0x00000002  // Tx reset
#define CSP_UART_CR_RXRES       0x00000001  // Rx reset


// Mode register bit definitions
#define CSP_UART_MR_CLKS                0x00000001  // Baud rate /8 pre-scalar
#define CSP_UART_MR_CHMODE_LLB          0x00000200  // Local loopback mode
#define CSP_UART_MR_CHMODE_NML          0x00000000  // Normal mode

#define CSP_UART_MR_CHRL_6              0x00000006  // 6 databits
#define CSP_UART_MR_CHRL_7              0x00000004  // 7 databits
#define CSP_UART_MR_CHRL_8              0x00000000  // 8 databits

#define CSP_UART_MR_PAR_NONE            0x00000020  // No parity mode
#define CSP_UART_MR_PAR_MARK            0x00000018  // Mark parity mode
#define CSP_UART_MR_PAR_SPACE           0x00000010  // Space parity mode
#define CSP_UART_MR_PAR_ODD             0x00000008  // Odd parity mode
#define CSP_UART_MR_PAR_EVEN            0x00000000  // Even parity mode

#define CSP_UART_MR_NBSTOP_1            0x00000000  // 1 stop bit
#define CSP_UART_MR_NBSTOP_2            0x00000080  // 2 stop bits

// Modem control register bit definitions
#define CSP_UART_MCR_DTR                0x00000001  // DTR control
#define CSP_UART_MCR_RTS                0x00000002  // RTS control
#define CSP_UART_MCR_FCM                0x00000020  // Auto flow control

// Modem status register bit definitions
#define CSP_UART_MSR_FCMS               0x00000100  // Auto flow control status
#define CSP_UART_MSR_DCD                0x00000080  // DCD status
#define CSP_UART_MSR_RI                 0x00000040  // RI status
#define CSP_UART_MSR_DSR                0x00000020  // DSR status
#define CSP_UART_MSR_CTS                0x00000010  // CTS status

// Channel status register bit definitions
#define CSP_UART_CSR_REMPTY             0x00000002  // Rx FIFO empty
#define CSP_UART_CSR_TEMPTY             0x00000008  // Tx FIFO empty
#define CSP_UART_CSR_TFUL               0x00000010  // Tx FIFO full

#endif
