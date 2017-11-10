/** @file
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <PiPei.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesLib.h>
#include <Platform/DramInfo.h>
#include <Ppi/DramInfo.h>
#include <Ppi/EmbeddedGpio.h>
#include <Ppi/MemoryDiscovered.h>

STATIC
CONST DRAM_INFO *mDramInfo = (VOID *)(UINTN)FixedPcdGet64 (PcdDramInfoBase);

/**
  Retrieve the number of discontiguous DRAM regions

  @param[out] RegionCount       The number of available DRAM regions

  @retval EFI_SUCCESS           The data was successfully returned.
  @retval EFI_INVALID_PARAMETER RegionCount == NULL

**/
STATIC
EFI_STATUS
EFIAPI
GetDramRegionCount (
  OUT   UINTN                 *RegionCount
  )
{
  if (RegionCount == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *RegionCount = mDramInfo->NumRegions;

  return EFI_SUCCESS;
}

/**
  Retrieve the base and size of a DRAM region

  @param[in]  RegionIndex       The 0-based index of the region to retrieve
  @param[out] Base              The base of the requested region
  @param[out] Size              The size of the requested region

  @retval EFI_SUCCESS           The data was successfully returned.
  @retval EFI_INVALID_PARAMETER Base == NULL or Size == NULL
  @retval EFI_NOT_FOUND         No region exists with index >= RegionIndex

**/
STATIC
EFI_STATUS
EFIAPI
GetDramRegion (
  IN    UINTN                 RegionIndex,
  OUT   UINT64                *Base,
  OUT   UINT64                *Size
  )
{
  if (Base == NULL || Size == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (RegionIndex >= mDramInfo->NumRegions) {
    return EFI_NOT_FOUND;
  }

  *Base = mDramInfo->Entry[RegionIndex].Base;
  *Size = mDramInfo->Entry[RegionIndex].Size;

  return EFI_SUCCESS;
}

STATIC SYNQUACER_DRAM_INFO_PPI mDramInfoPpi = {
  GetDramRegionCount,
  GetDramRegion
};

STATIC CONST EFI_PEI_PPI_DESCRIPTOR mDramInfoPpiDescriptor = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gSynQuacerDramInfoPpiGuid,
  &mDramInfoPpi
};

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  )
{
  EMBEDDED_GPIO_PPI   *Gpio;
  EFI_STATUS          Status;
  UINTN               Value;

  ASSERT (mDramInfo->NumRegions > 0);

  Status = PeiServicesLocatePpi (&gEdkiiEmbeddedGpioPpiGuid, 0, NULL,
             (VOID **)&Gpio);
  ASSERT_EFI_ERROR (Status);

  Status = Gpio->Set (Gpio, FixedPcdGet32 (PcdClearSettingsGpioPin),
                   GPIO_MODE_INPUT);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "%a: failed to set GPIO as input - %r\n", __FUNCTION__,
      Status));
  } else {
    Status = Gpio->Get (Gpio, FixedPcdGet32 (PcdClearSettingsGpioPin), &Value);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "%a: failed to get GPIO state - %r\n", __FUNCTION__,
        Status));
    } else if (Value > 0) {
      DEBUG ((DEBUG_INFO, "%a: clearing NVRAM\n", __FUNCTION__));
      PeiServicesSetBootMode (BOOT_WITH_DEFAULT_SETTINGS);
    }
  }

  //
  // Record the first region into PcdSystemMemoryBase and PcdSystemMemorySize.
  // This is the region we will use for UEFI itself.
  //
  Status = PcdSet64S (PcdSystemMemoryBase, mDramInfo->Entry[0].Base);
  ASSERT_EFI_ERROR (Status);

  Status = PcdSet64S (PcdSystemMemorySize, mDramInfo->Entry[0].Size);
  ASSERT_EFI_ERROR (Status);

  BuildFvHob (FixedPcdGet64 (PcdFvBaseAddress), FixedPcdGet32 (PcdFvSize));

  return PeiServicesInstallPpi (&mDramInfoPpiDescriptor);
}
