/** I2c.h
  Header defining the constant, base address amd function for I2C controller

  Copyright 2017 NXP

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_H___
#define __I2C_H__

#include <Uefi.h>

#define I2C1  0
#define I2C2  1
#define I2C3  2
#define I2C4  3

///
/// Define the I2C flags
///
#define I2C_READ_FLAG         0x1
#define I2C_WRITE_FLAG        0x2

/**
  Function to initialize i2c bus

  @param   I2cBus     I2c Controller number
  @param   Speed      Value to be set

  @retval  EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
I2cBusInit (
  IN   UINT32   I2cBus,
  IN   UINT32    Speed
  );

/**
  Function to read data usin i2c

  @param   I2cBus     I2c Controller number
  @param   Chip       Address of slave device from where data to be read
  @param   Offset     Offset of slave memory
  @param   Alen       Address length of slave
  @param   Buffer     A pointer to the destination buffer for the data
  @param   Len        Length of data to be read

  @retval  EFI_NOT_READY    Arbitration lost
  @retval  EFI_TIMEOUT      Failed to initialize data transfer in predefined time
  @retval  EFI_NOT_FOUND    ACK was not recieved
  @retval  EFI_SUCCESS      Read was successful

**/
EFI_STATUS
I2cDataRead (
  IN   UINT32  I2cBus,
  IN   UINT8    Chip,
  IN   UINT32  Offset,
  IN   UINT32  Alen,
  OUT  UINT8  *Buffer,
  IN   UINT32  Len
  );

/**
  Function to write data using i2c bus

  @param   I2cBus                  I2c Controller number
  @param   Chip                    Address of slave device where data to be written
  @param   Offset                  Offset of slave memory
  @param   Alen                    Address length of slave
  @param   Buffer                  A pointer to the source buffer for the data
  @param   Len                     Length of data to be write

  @retval  EFI_NOT_READY           Arbitration lost
  @retval  EFI_TIMEOUT             Failed to initialize data transfer in predefined time
  @retval  EFI_NOT_FOUND           ACK was not recieved
  @retval  EFI_SUCCESS             Read was successful

**/
EFI_STATUS
I2cDataWrite (
  IN   UINT32        I2cBus,
  IN   UINT8         Chip,
  IN   UINT32        Offset,
  IN   INT32         Alen,
  OUT  UINT8         *Buffer,
  IN   INT32         Len
  );

/**
  Function to reset I2c
  @param   I2cBus                    I2c Controller number

  @return  VOID
**/
VOID
I2cReset (
  UINT32 I2cBus
  );

/**
  Function to Probe I2c slave device
  @param    I2cBus                    I2c Controller number

  @retval   EFI_INVALID_PARAMETER     Invalid I2c Controller number
  @retval   EFI_SUCCESS               I2c device probed successfully

**/
EFI_STATUS
EFIAPI
I2cProbeDevices (
  IN   INT16    I2cBus,
  IN   UINT8    Chip
  );

#endif
