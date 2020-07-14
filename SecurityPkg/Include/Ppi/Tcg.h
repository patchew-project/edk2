/** @file
  TCG PPI services.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _TCG_PPI_H_
#define _TCG_PPI_H_

#include <IndustryStandard/UefiTcgPlatform.h>

typedef struct _EDKII_TCG_PPI EDKII_TCG_PPI;

/**
  Tpm measure and log data, and extend the measurement result into a specific PCR.

  @param[in]      This          Indicates the calling context
  @param[in]      HashData      Physical address of the start of the data buffer
                                to be hashed, extended, and logged.
  @param[in]      HashDataLen   The length, in bytes, of the buffer referenced by HashData.
  @param[in]      NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.
  @param[in]      NewEventData  Pointer to the new event data.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_UNSUPPORTED       TPM device not available.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.
**/
typedef
EFI_STATUS
(EFIAPI *EDKII_TCG_HASH_LOG_EXTEND_EVENT)(
  IN      EDKII_TCG_PPI             *This,
  IN      UINT8                     *HashData,
  IN      UINTN                     HashDataLen,
  IN      TCG_PCR_EVENT_HDR         *NewEventHdr,
  IN      UINT8                     *NewEventData
  );

///
/// The EFI_TCG Protocol abstracts TCG activity.
///
struct _EDKII_TCG_PPI {
  EDKII_TCG_HASH_LOG_EXTEND_EVENT     HashLogExtendEvent;
};

extern EFI_GUID gEdkiiTcgPpiGuid;

#endif
