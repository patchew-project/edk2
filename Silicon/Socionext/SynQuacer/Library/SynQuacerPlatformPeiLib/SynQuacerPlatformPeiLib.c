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

STATIC
EFI_STATUS
EFIAPI
PeiMemoryDiscoveredNotify (
  IN EFI_PEI_SERVICES          **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
  IN VOID                      *Ppi
  )
{
  EFI_FIRMWARE_VOLUME_HEADER  *Fvh;
  VOID                        *Buf;

  Fvh = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)FixedPcdGet64 (PcdSecondaryFvBase);

  Buf = AllocatePages (EFI_SIZE_TO_PAGES (Fvh->FvLength));
  if (Buf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((DEBUG_INFO, "%a: copying secondary FV to DRAM\n", __FUNCTION__));
  CopyMem (Buf, Fvh, Fvh->FvLength);
  DEBUG ((DEBUG_INFO, "%a: copying done\n", __FUNCTION__));

  PeiServicesInstallFvInfoPpi (NULL, Buf, Fvh->FvLength, NULL, NULL);

  return EFI_SUCCESS;
}

STATIC CONST EFI_PEI_NOTIFY_DESCRIPTOR mPeiMemoryDiscoveredNotifyDesc = {
  EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK |
  EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gEfiPeiMemoryDiscoveredPpiGuid,
  PeiMemoryDiscoveredNotify
};

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  )
{
  EFI_STATUS      Status;

  ASSERT (mDramInfo->NumRegions > 0);

  //
  // Record the first region into PcdSystemMemoryBase and PcdSystemMemorySize.
  // This is the region we will use for UEFI itself.
  //
  Status = PcdSet64S (PcdSystemMemoryBase, mDramInfo->Entry[0].Base);
  ASSERT_EFI_ERROR (Status);

  Status = PcdSet64S (PcdSystemMemorySize, mDramInfo->Entry[0].Size);
  ASSERT_EFI_ERROR (Status);

  BuildFvHob (FixedPcdGet64 (PcdFvBaseAddress), FixedPcdGet32 (PcdFvSize));

  Status = PeiServicesNotifyPpi (&mPeiMemoryDiscoveredNotifyDesc);
  ASSERT_EFI_ERROR (Status);

  return PeiServicesInstallPpi (&mDramInfoPpiDescriptor);
}
