/** @file
  Serial Port Library for Cadence IP6528 UART.
  Copyright (c) 2015-2017, Cadence Design Systems, Inc. All rights reserved.

  Based on:

  Null Serial Port library instance with empty functions.

  Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/CspSerialPortLib.h>
#include <Library/CspSysReg.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>


RETURN_STATUS
EFIAPI
CspUartInitializePort (
  IN     UINTN               UartBase,
  IN OUT UINT64              *BaudRate,
  IN OUT EFI_PARITY_TYPE     *Parity,
  IN OUT UINT8               *DataBits,
  IN OUT EFI_STOP_BITS_TYPE  *StopBits
);
VOID CspUartPutChar (IN UINTN UartBase, IN UINT8 Char);
UINT8 CspUartGetChar (IN UINTN UartBase);

/**
  Initialize the serial device hardware.

  If no initialization is required, then return RETURN_SUCCESS.
  If the serial device was successfully initialized, then return RETURN_SUCCESS.
  If the serial device could not be initialized, then return RETURN_DEVICE_ERROR.

  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_DEVICE_ERROR   The serial device could not be initialized.

**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  UINT64              BaudRate;
  EFI_PARITY_TYPE     Parity;
  UINT8               DataBits;
  EFI_STOP_BITS_TYPE  StopBits;

  BaudRate = FixedPcdGet64 (PcdUartDefaultBaudRate);
  Parity = (EFI_PARITY_TYPE)FixedPcdGet8 (PcdUartDefaultParity);
  DataBits = FixedPcdGet8 (PcdUartDefaultDataBits);
  StopBits = (EFI_STOP_BITS_TYPE)FixedPcdGet8 (PcdUartDefaultStopBits);

  return CspUartInitializePort (
           (UINTN)FixedPcdGet64 (PcdCspSerialBase),
           &BaudRate,
           &Parity,
           &DataBits,
           &StopBits
           );
}

/**
  Set new attributes to UART.

  @param  BaudRate                The baud rate of the serial device. If the
                                  baud rate is not supported, the speed will
                                  be reduced down to the nearest supported one
                                  and the variable's value will be updated
                                  accordingly.
  @param  ReceiveFifoDepth        The number of characters the device will
                                  buffer on input. If the specified value is
                                  not supported, the variable's value will
                                  be reduced down to the nearest supported one.
  @param  Timeout                 If applicable, the number of microseconds the
                                  device will wait before timing out a Read or
                                  a Write operation.
  @param  Parity                  If applicable, this is the EFI_PARITY_TYPE
                                  that is computed or checked as each character
                                  is transmitted or received. If the device
                                  does not support parity, the value is the
                                  default parity value.
  @param  DataBits                The number of data bits in each character
  @param  StopBits                If applicable, the EFI_STOP_BITS_TYPE number
                                  of stop bits per character. If the device
                                  does not support stop bits, the value is the
                                  default stop bit value.

  @retval EFI_SUCCESS             All attributes were set correctly.
  @retval EFI_INVALID_PARAMETERS  One or more attributes has an unsupported
                                  value.

**/
RETURN_STATUS
EFIAPI
SerialPortSetAttributes (
  IN OUT UINT64              *BaudRate,
  IN OUT UINT32              *ReceiveFifoDepth,
  IN OUT UINT32              *Timeout,
  IN OUT EFI_PARITY_TYPE     *Parity,
  IN OUT UINT8               *DataBits,
  IN OUT EFI_STOP_BITS_TYPE  *StopBits
  )
{
  return CspUartInitializePort (
           (UINTN)FixedPcdGet64 (PcdCspSerialBase),
           BaudRate,
           Parity,
           DataBits,
           StopBits
           );
}

/**
  Write data from buffer to serial device.

  Writes NumberOfBytes data bytes from Buffer to the serial device.
  The number of bytes actually written to the serial device is returned.
  If the return value is less than NumberOfBytes, then the write operation failed.
  If Buffer is NULL, then ASSERT().
  If NumberOfBytes is zero, then return 0.

  @param  Buffer           The pointer to the data buffer to be written.
  @param  NumberOfBytes    The number of bytes to written to the serial device.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes written to the serial device.
                           If this value is less than NumberOfBytes, then the read operation failed.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
)
{
  UINTN i;
  for (i = 0; i <  NumberOfBytes; i++) {
    CspUartPutChar ((UINTN)PcdGet64 (PcdCspSerialBase), Buffer[i]);
  }
  return i;
}


/**
  Read data from serial device and save the datas in buffer.

  Reads NumberOfBytes data bytes from a serial device into the buffer
  specified by Buffer. The number of bytes actually read is returned.
  If the return value is less than NumberOfBytes, then the rest operation failed.
  If Buffer is NULL, then ASSERT().
  If NumberOfBytes is zero, then return 0.

  @param  Buffer           The pointer to the data buffer to store the data read from the serial device.
  @param  NumberOfBytes    The number of bytes which will be read.

  @retval 0                Read data failed; No data is to be read.
  @retval >0               The actual number of bytes read from serial device.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{
  UINTN i;
  for (i = 0; i < NumberOfBytes; i++) {
    Buffer[i] = CspUartGetChar ((UINTN)PcdGet64 (PcdCspSerialBase));
  }
  return i;
}

/**
  Polls a serial device to see if there is any data waiting to be read.

  Polls a serial device to see if there is any data waiting to be read.
  If there is data waiting to be read from the serial device, then TRUE is returned.
  If there is no data waiting to be read from the serial device, then FALSE is returned.

  @retval TRUE             Data is waiting to be read from the serial device.
  @retval FALSE            There is no data waiting to be read from the serial device.

**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  return (MmioRead32 ((UINTN)(PcdGet64 (PcdCspSerialBase + CSP_UART_CSR))) &
          CSP_UART_CSR_REMPTY) ? FALSE : TRUE;
}

/**

  Assert or deassert the control signals on a serial port.
  The following control signals are set according their bit settings :
  . Request to Send
  . Data Terminal Ready

  @param[in]  Control  The following bits are taken into account :
                       . EFI_SERIAL_REQUEST_TO_SEND : assert/deassert the
                         "Request To Send" control signal if this bit is
                         equal to one/zero.
                       . EFI_SERIAL_DATA_TERMINAL_READY : assert/deassert
                         the "Data Terminal Ready" control signal if this
                         bit is equal to one/zero.
                       . EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE : enable/disable
                         the hardware loopback if this bit is equal to
                         one/zero.
                       . EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE : not supported.
                       . EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE : enable/
                         disable the hardware flow control based on CTS (Clear
                         To Send) and RTS (Ready To Send) control signals.

  @retval  RETURN_SUCCESS      The new control bits were set on the device.
  @retval  RETURN_UNSUPPORTED  The device does not support this operation.

**/
RETURN_STATUS
EFIAPI
SerialPortSetControl (
  IN UINT32  Control
  )
{
  UINT32  Bits;

  if (Control & (EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE |
                 EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE )) {
    return RETURN_UNSUPPORTED;
  }

  Bits = MmioRead32 (PcdGet64 (PcdCspSerialBase) + CSP_UART_MCR);

  if (Control & EFI_SERIAL_REQUEST_TO_SEND) {
    Bits |= CSP_UART_MCR_RTS;
  } else {
    Bits &= ~CSP_UART_MCR_RTS;
  }

  if (Control & EFI_SERIAL_DATA_TERMINAL_READY) {
    Bits |= CSP_UART_MCR_DTR;
  } else {
    Bits &= ~CSP_UART_MCR_DTR;
  }

  if (Control & EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE) {
    Bits |= CSP_UART_MCR_FCM;
  } else {
    Bits &= CSP_UART_MCR_FCM;
  }

  MmioWrite32 ((PcdGet64 (PcdCspSerialBase) + CSP_UART_MCR), Bits);

  return RETURN_SUCCESS;
}

/**

  Retrieve the status of the control bits on a serial device.

  @param[out]  Control  Status of the control bits on a serial device :

                        . EFI_SERIAL_DATA_CLEAR_TO_SEND,
                          EFI_SERIAL_DATA_SET_READY,
                          EFI_SERIAL_RING_INDICATE,
                          EFI_SERIAL_CARRIER_DETECT,
                          EFI_SERIAL_REQUEST_TO_SEND,
                          EFI_SERIAL_DATA_TERMINAL_READY
                          are all related to the DTE (Data Terminal Equipment)
                          and DCE (Data Communication Equipment) modes of
                          operation of the serial device.
                        . EFI_SERIAL_INPUT_BUFFER_EMPTY : equal to one if the
                          receive buffer is empty, 0 otherwise.
                        . EFI_SERIAL_OUTPUT_BUFFER_EMPTY : equal to one if the
                          transmit buffer is empty, 0 otherwise.
                        . EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE : equal to one if
                          the hardware loopback is enabled (the output feeds
                          the receive buffer), 0 otherwise.
                        . EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE : equal to one
                          if a loopback is accomplished by software, else 0.
                        . EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE : equal to
                          one if the hardware flow control based on CTS (Clear
                          To Send) and RTS (Ready To Send) control signals is
                          enabled, 0 otherwise.

  @retval RETURN_SUCCESS  The control bits were read from the device.

**/
RETURN_STATUS
EFIAPI
SerialPortGetControl (
  OUT UINT32  *Control
  )
{
  UINT32      ModemStatusReg;
  UINT32      ModemCtrlReg;
  UINT32      ChanStatusReg;

  ModemCtrlReg = MmioRead32 ((UINTN)(PcdGet64 (PcdCspSerialBase) +
                                     CSP_UART_MCR));
  ModemStatusReg = MmioRead32 ((UINTN)(PcdGet64 (PcdCspSerialBase) +
                                       CSP_UART_MSR));
  ChanStatusReg = MmioRead32 ((UINTN)(PcdGet64 (PcdCspSerialBase) +
                                       CSP_UART_CSR));

  *Control = 0;

  if ((ModemStatusReg & CSP_UART_MSR_CTS) == CSP_UART_MSR_CTS) {
    *Control |= EFI_SERIAL_CLEAR_TO_SEND;
  }

  if ((ModemStatusReg & CSP_UART_MSR_DSR) == CSP_UART_MSR_DSR) {
    *Control |= EFI_SERIAL_DATA_SET_READY;
  }

  if ((ModemStatusReg & CSP_UART_MSR_RI) == CSP_UART_MSR_RI) {
    *Control |= EFI_SERIAL_RING_INDICATE;
  }

  if ((ModemStatusReg & CSP_UART_MSR_DCD) == CSP_UART_MSR_DCD) {
    *Control |= EFI_SERIAL_CARRIER_DETECT;
  }

  if ((ModemCtrlReg & CSP_UART_MCR_RTS) == CSP_UART_MCR_RTS) {
    *Control |= EFI_SERIAL_REQUEST_TO_SEND;
  }

  if ((ModemCtrlReg & CSP_UART_MCR_DTR) == CSP_UART_MCR_DTR) {
    *Control |= EFI_SERIAL_DATA_TERMINAL_READY;
  }

  if ((ChanStatusReg & CSP_UART_CSR_REMPTY) == CSP_UART_CSR_REMPTY) {
    *Control |= EFI_SERIAL_INPUT_BUFFER_EMPTY;
  }

  if ((ChanStatusReg & CSP_UART_CSR_TEMPTY) == CSP_UART_CSR_TEMPTY) {
    *Control |= EFI_SERIAL_OUTPUT_BUFFER_EMPTY;
  }

  if ((ModemCtrlReg & CSP_UART_MCR_FCM) == CSP_UART_MCR_FCM) {
    *Control |= EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE;
  }

  return RETURN_SUCCESS;
}


/**

  Initialise the serial port to the specified settings.
  The serial port is re-configured only if the specified settings
  are different from the current settings.
  All unspecified settings will be set to the default values.

  @param  UartBase                The base address of the serial device.
  @param  BaudRate                The baud rate of the serial device. If the
                                  baud rate is not supported, the speed will be
                                  reduced to the nearest supported one and the
                                  variable's value will be updated accordingly.
  @param  Parity                  If applicable, this is the EFI_PARITY_TYPE
                                  that is computed or checked as each character
                                  is transmitted or received. If the device
                                  does not support parity, the value is the
                                  default parity value.
  @param  DataBits                The number of data bits in each character.
  @param  StopBits                If applicable, the EFI_STOP_BITS_TYPE number
                                  of stop bits per character.
                                  If the device does not support stop bits, the
                                  value is the default stop bit value.

  @retval RETURN_SUCCESS            All attributes were set correctly on the
                                    serial device.
  @retval RETURN_INVALID_PARAMETER  One or more of the attributes has an
                                    unsupported value.

**/
RETURN_STATUS
EFIAPI
CspUartInitializePort (
  IN     UINTN               UartBase,
  IN OUT UINT64              *BaudRate,
  IN OUT EFI_PARITY_TYPE     *Parity,
  IN OUT UINT8               *DataBits,
  IN OUT EFI_STOP_BITS_TYPE  *StopBits
  )
{
  UINT32 RegVal = 0;
  UINT32 BaudDivisor = 0;

  // Wait for Tx FIFO to empty before initializing
  if (!(MmioRead32 (UartBase + CSP_UART_CR) & CSP_UART_CR_TXDIS)) {
    while (!(MmioRead32 (UartBase + CSP_UART_CSR) & CSP_UART_CSR_TEMPTY))
            ;
  }

  // Disable Tx/Rx before setting baud rate
  RegVal = MmioRead32 (UartBase + CSP_UART_CR);
  RegVal |= CSP_UART_CR_TXDIS | CSP_UART_CR_RXDIS;
  MmioWrite32 ((UartBase + CSP_UART_CR), RegVal);

  // Set baud rate
  UINT32 SelClk = MmioRead32 ((UINTN)(PcdGet64 (PcdCspSysRegBase) +
                                     CSP_SYSREG_CPU_FREQ));
  UINT32 BDiv = 0;

  if (SelClk < 0x1800000) {
    BaudDivisor = 1;
  } else {
    BaudDivisor = 8;
  }
  MmioWrite32 ((UartBase + CSP_UART_BRGR), BaudDivisor);
  BDiv = (SelClk + ((*BaudRate * BaudDivisor) / 2)) / (*BaudRate * BaudDivisor);
  MmioWrite32 ((UartBase + CSP_UART_BDIV), (BDiv - 1));

  // Reset and enable Tx/Rx
  RegVal = MmioRead32 (UartBase + CSP_UART_CR);
  RegVal &= ~(CSP_UART_CR_TXDIS | CSP_UART_CR_RXDIS);
  RegVal |= CSP_UART_CR_TXEN | CSP_UART_CR_TXRES | \
          CSP_UART_CR_RXEN | CSP_UART_CR_RXRES;;
  MmioWrite32 ((UartBase + CSP_UART_CR), RegVal);

  RegVal = MmioRead32 (UartBase + CSP_UART_MR) & 1;

  //
  // Data Bits
  //
  switch (*DataBits) {
  case 0:
    *DataBits = 8;
  case 8:
    RegVal |= CSP_UART_MR_CHRL_8;
    break;
  case 7:
    RegVal |= CSP_UART_MR_CHRL_7;
    break;
  case 6:
    RegVal |= CSP_UART_MR_CHRL_6;
    break;
  default:
    return RETURN_INVALID_PARAMETER;
  }

  //
  // Stop Bits
  //
  switch (*StopBits) {
  case DefaultStopBits:
    *StopBits = OneStopBit;
  case OneStopBit:
    RegVal |= CSP_UART_MR_NBSTOP_1;
    break;
  case TwoStopBits:
    RegVal |= CSP_UART_MR_NBSTOP_2;
    break;
  default:
    return RETURN_INVALID_PARAMETER;
  }

  //
  // Parity
  //
  switch (*Parity) {
  case DefaultParity:
    *Parity = NoParity;
  case NoParity:
    RegVal |= CSP_UART_MR_PAR_NONE;
    break;
  case EvenParity:
    RegVal |= CSP_UART_MR_PAR_EVEN;
    break;
  case OddParity:
    RegVal |= CSP_UART_MR_PAR_ODD;
    break;
  case MarkParity:
    RegVal |= CSP_UART_MR_PAR_MARK;
    break;
  case SpaceParity:
    RegVal |= CSP_UART_MR_PAR_SPACE;
    break;
  default:
    return RETURN_INVALID_PARAMETER;
  }

  MmioWrite32 ((UartBase + CSP_UART_MR), RegVal);

  return RETURN_SUCCESS;
}

VOID CspUartPutChar (IN UINTN UartBase, IN UINT8 Char)
{
  while ((MmioRead32 (UartBase + CSP_UART_CSR) & CSP_UART_CSR_TFUL)
         == CSP_UART_CSR_TFUL)
          ;
  MmioWrite8 (UartBase + CSP_UART_FIFO, Char);
}

UINT8 CspUartGetChar (IN UINTN UartBase)
{
  while ((MmioRead32 (UartBase + CSP_UART_CSR) & CSP_UART_CSR_REMPTY)
         == CSP_UART_CSR_REMPTY)
          ;
  return MmioRead8 (UartBase + CSP_UART_FIFO);
}

