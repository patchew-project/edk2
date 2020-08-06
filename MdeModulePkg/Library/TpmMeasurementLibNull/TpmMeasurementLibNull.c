/** @file
  This library is used by other modules to measure data to TPM.

Copyright (c) 2015 - 2020, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

/**
  Tpm measure and log data, and extend the measurement result into a specific PCR.

  @param[in]  PcrIndex         PCR Index.
  @param[in]  EventType        Event type.
  @param[in]  EventLog         Measurement event log.
  @param[in]  LogLen           Event log length in bytes.
  @param[in]  HashData         The start of the data buffer to be hashed, extended.
  @param[in]  HashDataLen      The length, in bytes, of the buffer referenced by HashData

  @retval EFI_SUCCESS               Operation completed successfully.
  @retval EFI_UNSUPPORTED       TPM device not available.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.
**/
EFI_STATUS
EFIAPI
TpmMeasureAndLogData (
  IN UINT32             PcrIndex,
  IN UINT32             EventType,
  IN VOID               *EventLog,
  IN UINT32             LogLen,
  IN VOID               *HashData,
  IN UINT64             HashDataLen
  )
{
  //
  // Do nothing, just return EFI_SUCCESS.
  //
  return EFI_SUCCESS;
}

/**
  Mesure a FirmwareBlob.

  @param[in]  Descrption              Description for this FirmwareBlob.
  @param[in]  FirmwareBlobBase        Base address of this FirmwareBlob.
  @param[in]  FirmwareBlobLength      Size in bytes of this FirmwareBlob.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_UNSUPPORTED       TPM device not available.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.
*/
EFI_STATUS
EFIAPI
MeasureFirmwareBlob (
  IN UINT32                         PcrIndex,
  IN CHAR8                          *Description OPTIONAL,
  IN EFI_PHYSICAL_ADDRESS           FirmwareBlobBase,
  IN UINT64                         FirmwareBlobLength
  )
{
  //
  // Do nothing, just return EFI_SUCCESS.
  //
  return EFI_SUCCESS;
}

/**
  Mesure a HandoffTable.

  @param[in]  PcrIndex                PcrIndex of the measurment.
  @param[in]  Descrption              Description for this HandoffTable.
  @param[in]  TableGuid               GUID of this HandoffTable.
  @param[in]  TableAddress            Base address of this HandoffTable.
  @param[in]  TableLength             Size in bytes of this HandoffTable.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_UNSUPPORTED       TPM device not available.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.
*/
EFI_STATUS
EFIAPI
MeasureHandoffTable (
  IN UINT32                         PcrIndex,
  IN CHAR8                          *Description OPTIONAL,
  IN EFI_GUID                       *TableGuid,
  IN VOID                           *TableAddress,
  IN UINTN                          TableLength
  )
{
  //
  // Do nothing, just return EFI_SUCCESS.
  //
  return EFI_SUCCESS;
}
