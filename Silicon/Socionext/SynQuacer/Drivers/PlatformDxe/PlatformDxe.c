/** @file
  SynQuacer DXE platform driver.

  Copyright (c) 2017, Linaro, Ltd. All rights reserved.<BR>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DtPlatformDtbLoaderLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Platform/MemoryMap.h>
#include <Protocol/NonDiscoverableDevice.h>

STATIC EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mNetsecDesc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    SYNQUACER_NETSEC1_BASE,                           // AddrRangeMin
    SYNQUACER_NETSEC1_BASE +
    SYNQUACER_NETSEC1_BASE_SZ - 1,                    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    SYNQUACER_NETSEC1_BASE_SZ,                        // AddrLen
  }, {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet32 (PcdNetsecEepromBase),              // AddrRangeMin
    FixedPcdGet32 (PcdNetsecEepromBase) +
    SYNQUACER_EEPROM_BASE_SZ - 1,                     // AddrRangeMax
    0,                                                // AddrTranslationOffset
    SYNQUACER_EEPROM_BASE_SZ,                         // AddrLen
  }, {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet32 (PcdNetsecPhyAddress),              // AddrRangeMin
    FixedPcdGet32 (PcdNetsecPhyAddress),              // AddrRangeMax
    0,                                                // AddrTranslationOffset
    1,                                                // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

STATIC
EFI_STATUS
RegisterNetsec (
  VOID
  )
{
  NON_DISCOVERABLE_DEVICE             *Device;
  EFI_STATUS                          Status;
  EFI_HANDLE                          Handle;

  Device = (NON_DISCOVERABLE_DEVICE *)AllocateZeroPool (sizeof (*Device));
  if (Device == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Device->Type = &gNetsecNonDiscoverableDeviceGuid;
  Device->DmaType = NonDiscoverableDeviceDmaTypeNonCoherent;
  Device->Resources = mNetsecDesc;

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (&Handle,
                  &gEdkiiNonDiscoverableDeviceProtocolGuid, Device,
                  NULL);
  if (EFI_ERROR (Status)) {
    goto FreeDevice;
  }
  return EFI_SUCCESS;

FreeDevice:
  FreePool (Device);

  return Status;
}

EFI_STATUS
EFIAPI
PlatformDxeEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                      Status;
  VOID                            *Dtb;
  UINTN                           DtbSize;

  Dtb = NULL;
  Status = DtPlatformLoadDtb (&Dtb, &DtbSize);
  if (!EFI_ERROR (Status)) {
    Status = gBS->InstallConfigurationTable (&gFdtTableGuid, Dtb);
  }
  if (EFI_ERROR (Status)) {
     DEBUG ((DEBUG_ERROR,
      "%a: failed to install FDT configuration table - %r\n", __FUNCTION__,
      Status));
  }

  return RegisterNetsec ();
}
