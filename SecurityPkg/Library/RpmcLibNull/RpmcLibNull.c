/** @file
  NULL RpmcLib instance for build purpose.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/RpmcLib.h>

/**
  Requests the current monotonic counter from the designated RPMC counter.

  @param[in]    CounterIndex            The RPMC index.
  @param[out]   CounterValue            A pointer to a buffer to store the RPMC value.

  @retval       EFI_SUCCESS             The operation completed successfully.
  @retval       EFI_INVALID_PARAMETER   The CounterValue pointer is NULL or CounterIndex is invalid.
  @retval       EFI_NOT_READY           The given RPMC at CounterIndex is not yet initialized.
  @retval       EFI_DEVICE_ERROR        A device error occurred while attempting to update the counter.
  @retval       EFI_UNSUPPORTED         Requesting the monotonic counter is not supported in the current boot configuration.
**/
EFI_STATUS
EFIAPI
RequestMonotonicCounter (
  IN  UINT8   CounterIndex,
  OUT UINT32  *CounterValue
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

/**
  Increments the designated monotonic counter in the SPI flash device by 1.

  @param[in]    CounterIndex            The RPMC index.

  @retval       EFI_SUCCESS             The operation completed successfully.
  @retval       EFI_INVALID_PARAMETER   The given CounterIndex value is invalid.
  @retval       EFI_NOT_READY           The given RPMC at CounterIndex is not yet initialized.
  @retval       EFI_DEVICE_ERROR        A device error occurred while attempting to update the counter.
  @retval       EFI_UNSUPPORTED         Incrementing the monotonic counter is not supported in the current boot configuration.
**/
EFI_STATUS
EFIAPI
IncrementMonotonicCounter (
  IN  UINT8   CounterIndex
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

