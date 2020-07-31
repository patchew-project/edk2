/** @file
  This library is used by FSP modules to measure data to TPM.

Copyright (c) 2020, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/TpmMeasurementLib.h>
#include <Library/FspMeasurementLib.h>

#include <IndustryStandard/UefiTcgPlatform.h>

/**
  Mesure a FSP FirmwareBlob.

  @param[in]  PcrIndex                PCR Index.
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
MeasureFspFirmwareBlob (
  IN UINT32                         PcrIndex,
  IN CHAR8                          *Description OPTIONAL,
  IN EFI_PHYSICAL_ADDRESS           FirmwareBlobBase,
  IN UINT64                         FirmwareBlobLength
  )
{
  return MeasureFirmwareBlob (PcrIndex, Description, FirmwareBlobBase, FirmwareBlobLength);
}

/**
  Mesure a FSP FirmwareBlob.

  @param[in]  Descrption              Description for this FirmwareBlob.
  @param[in]  FirmwareBlobBase        Base address of this FirmwareBlob.
  @param[in]  FirmwareBlobLength      Size in bytes of this FirmwareBlob.
  @param[in]  CfgRegionOffset         Configuration region offset in bytes.
  @param[in]  CfgRegionSize           Configuration region in bytes.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_UNSUPPORTED       TPM device not available.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.
*/
EFI_STATUS
EFIAPI
MeasureFspFirmwareBlobWithCfg (
  IN CHAR8                          *Description OPTIONAL,
  IN EFI_PHYSICAL_ADDRESS           FirmwareBlobBase,
  IN UINT64                         FirmwareBlobLength,
  IN UINT32                         CfgRegionOffset,
  IN UINT32                         CfgRegionSize
  )
{
  return MeasureFirmwareBlobWithCfg (Description, FirmwareBlobBase, FirmwareBlobLength, CfgRegionOffset, CfgRegionSize);

}

