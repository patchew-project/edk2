/** @file
  This file encapsulate the usage of PCI Platform Protocol

  This file define the necessary hooks used to obtain the platform
  level data and policies which could be used in the PCI Enumeration phases

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PciPlatformSupport.h"

EFI_PCI_PLATFORM_PROTOCOL                     *mPciPlatformProtocol;
EFI_PCI_OVERRIDE_PROTOCOL                     *mPciOverrideProtocol;

EFI_PCI_PLATFORM_PROTOCOL2                    *mPciPlatformProtocol2;
EFI_PCI_OVERRIDE_PROTOCOL2                    *mPciOverrideProtocol2;


/**
  This function retrieves the PCI Platform Protocol published by platform driver

**/
VOID
GetPciPlatformProtocol (
  )
{
  mPciPlatformProtocol2 = NULL;
  gBS->LocateProtocol (
      &gEfiPciPlatformProtocol2Guid,
      NULL,
      (VOID **) &mPciPlatformProtocol2
  );

  //
  // If PCI Platform protocol doesn't exist, try to get Pci Override Protocol.
  //
  if (mPciPlatformProtocol2 == NULL) {
    mPciOverrideProtocol2 = NULL;
    gBS->LocateProtocol (
        &gEfiPciOverrideProtocol2Guid,
        NULL,
        (VOID **) &mPciOverrideProtocol2
    );
  }
  //
  // fetch the old PCI Platform Protocols if new are not installed
  //
  if (mPciOverrideProtocol2 == NULL) {

    mPciPlatformProtocol = NULL;
    gBS->LocateProtocol (
        &gEfiPciPlatformProtocolGuid,
        NULL,
        (VOID **) &mPciPlatformProtocol
    );

    //
    // If PCI Platform protocol doesn't exist, try to  get Pci Override Protocol.
    //
    if (mPciPlatformProtocol == NULL) {
      mPciOverrideProtocol = NULL;
      gBS->LocateProtocol (
          &gEfiPciOverrideProtocolGuid,
          NULL,
          (VOID **) &mPciOverrideProtocol
      );
    }
  }
}

/**
  This function indicates the presence of PCI Platform driver
  @retval     TRUE or FALSE
**/
BOOLEAN
CheckPciPlatformProtocolInstall (
  )
{
  if (mPciPlatformProtocol2 != NULL) {
    return TRUE;
  } else if (mPciOverrideProtocol2 != NULL) {
    return TRUE;
  } else {
    if (mPciPlatformProtocol != NULL) {
      return TRUE;
    } else if (mPciOverrideProtocol != NULL){
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Provides the hooks from the PCI bus driver to every PCI controller (device/function) at various
  stages of the PCI enumeration process that allow the host bridge driver to preinitialize individual
  PCI controllers before enumeration.

  This function is called during the PCI enumeration process. No specific action is expected from this
  member function. It allows the host bridge driver to preinitialize individual PCI controllers before
  enumeration.

  @param[in] HostBridgeHandle     The associated PCI host bridge handle.
  @param[in] RootBridgeHandle     The associated PCI root bridge handle.
  @param[in] RootBridgePciAddress The address of the PCI device on the PCI bus.
  @param[in] Phase          The phase of the PCI controller enumeration.
  @param[in] ExecPhase      Defines the execution phase of the PCI chipset driver.

  @retval    Status         returns the status from the PCI Platform protocol as is

**/
EFI_STATUS
PciPlatformPreprocessController (
  IN EFI_HANDLE                                    HostBridgeHandle,
  IN EFI_HANDLE                                    RootBridgeHandle,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS   RootBridgePciAddress,
  IN EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE  Phase,
  IN EFI_PCI_EXECUTION_PHASE                       ExecPhase
  )
{
  EFI_STATUS  Status;

  if (mPciPlatformProtocol2 != NULL) {
    //
    // Call PlatformPci::PrepController() if the protocol is present.
    //
    Status = mPciPlatformProtocol2->PlatformPrepController (
                                      (EFI_PCI_PLATFORM_PROTOCOL*)mPciPlatformProtocol2,
                                      HostBridgeHandle,
                                      RootBridgeHandle,
                                      RootBridgePciAddress,
                                      Phase,
                                      ExecPhase
                                    );
  } else if (mPciOverrideProtocol2 != NULL) {
    //
    // Call PlatformPci::PrepController() if the protocol is present.
    //
    Status = mPciOverrideProtocol2->PlatformPrepController (
                                      (EFI_PCI_PLATFORM_PROTOCOL*)mPciOverrideProtocol2,
                                      HostBridgeHandle,
                                      RootBridgeHandle,
                                      RootBridgePciAddress,
                                      Phase,
                                      ExecPhase
                                    );
  } else {
    if (mPciPlatformProtocol != NULL) {
      //
      // Call PlatformPci::PrepController() if the protocol is present.
      //
      Status = mPciPlatformProtocol->PlatformPrepController (
                                      mPciPlatformProtocol,
                                      HostBridgeHandle,
                                      RootBridgeHandle,
                                      RootBridgePciAddress,
                                      Phase,
                                      ExecPhase
                                    );
    } else if (mPciOverrideProtocol != NULL) {
      //
      // Call PlatformPci::PrepController() if the protocol is present.
      //
      Status = mPciOverrideProtocol->PlatformPrepController (
                                      mPciOverrideProtocol,
                                      HostBridgeHandle,
                                      RootBridgeHandle,
                                      RootBridgePciAddress,
                                      Phase,
                                      ExecPhase
                                    );
    } else {
      //
      // return PCI Platform Protocol not found
      //
      return EFI_NOT_FOUND;
    }
  }
  return Status;
}

/**
  This function notifies the PCI Platform driver about the PCI host bridge resource
  allocation phase and PCI execution phase.

  @param[in]  HostBridge     The handle of the host bridge controller.
  @param[in]  Phase          The phase of the PCI bus enumeration.
  @param[in]  ExecPhase      Defines the execution phase of the PCI chipset driver.
  @retval     Status          returns the status from the PCI Platform protocol as is

**/
EFI_STATUS
PciPlatformNotifyPhase (
  IN  EFI_HANDLE                                      HostBridgeHandle,
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE   Phase,
  IN  EFI_PCI_EXECUTION_PHASE                         ExecPhase
  )
{
  EFI_STATUS  Status;

  if ( mPciPlatformProtocol2 != NULL) {
    Status = mPciPlatformProtocol2->PlatformNotify (
                                      (EFI_PCI_PLATFORM_PROTOCOL*)mPciPlatformProtocol2,
                                      HostBridgeHandle,
                                      Phase,
                                      ExecPhase
                                    );
  } else if ( mPciOverrideProtocol2 != NULL) {
    Status = mPciOverrideProtocol2->PlatformNotify (
                                      (EFI_PCI_PLATFORM_PROTOCOL*)mPciOverrideProtocol2,
                                      HostBridgeHandle,
                                      Phase,
                                      ExecPhase
                                    );
  } else {

    if ( mPciPlatformProtocol != NULL) {
      Status = mPciPlatformProtocol->PlatformNotify (
                                      mPciPlatformProtocol,
                                      HostBridgeHandle,
                                      Phase,
                                      ExecPhase
                                    );
    } else if ( mPciOverrideProtocol != NULL){
      Status = mPciOverrideProtocol->PlatformNotify (
                                      mPciOverrideProtocol,
                                      HostBridgeHandle,
                                      Phase,
                                      ExecPhase
                                    );
    } else {
      //
      // return PCI Platform Protocol not found
      //
      return EFI_NOT_FOUND;
    }
  }
  return Status;
}

/**
  This function retrieves the PCI platform policy.

  @param  PciPolicy     pointer to the legacy EFI_PCI_PLATFORM_POLICY
  @retval Status        returns the status from the PCI Platform protocol as is

**/
EFI_STATUS
PciGetPlatformPolicy (
  OUT EFI_PCI_PLATFORM_POLICY *PciPolicy
  )
{
  EFI_STATUS  Status;

  if ( mPciPlatformProtocol2 != NULL) {
      Status = mPciPlatformProtocol2->GetPlatformPolicy (
                                        (EFI_PCI_PLATFORM_PROTOCOL*)mPciPlatformProtocol2,
                                        PciPolicy
                                      );
  } else if ( mPciOverrideProtocol2 != NULL) {
      Status = mPciOverrideProtocol2->GetPlatformPolicy (
                                        (EFI_PCI_PLATFORM_PROTOCOL*)mPciOverrideProtocol2,
                                        PciPolicy
                                      );
  } else {
    if ( mPciPlatformProtocol != NULL) {
      Status = mPciPlatformProtocol->GetPlatformPolicy (
                                      mPciPlatformProtocol,
                                      PciPolicy
                                    );
    }

    if ( mPciOverrideProtocol != NULL) {
      Status = mPciOverrideProtocol->GetPlatformPolicy (
                                      mPciOverrideProtocol,
                                      PciPolicy
                                    );
    } else {
      //
      // return PCI Platform Protocol not found
      //
      return EFI_NOT_FOUND;
    }
  }
  return Status;
}

/**
  This function retrieves the Option ROM image and size from the Platform.

  It uses the PCI_IO_DEVICE internal fields are used to store OpROM image/size

  @param Controller     An EFI handle for the PCI bus controller.
  @param PciIoDevice    A PCI_IO_DEVICE pointer to the PCI IO device to be registered.

  @retval EFI_SUCCESS            The option ROM was available for this device and loaded into memory.
  @retval EFI_NOT_FOUND          No option ROM was available for this device.
  @retval EFI_OUT_OF_RESOURCES   No memory was available to load the option ROM.
  @retval EFI_DEVICE_ERROR       An error occurred in obtaining the option ROM.

**/
EFI_STATUS
GetPlatformPciOptionRom (
  IN  EFI_HANDLE                    Controller,
  IN  PCI_IO_DEVICE                 *PciIoDevice
  )
{
  EFI_STATUS  Status;
  VOID        *PlatformOpRomBuffer;
  UINTN       PlatformOpRomSize;

  if (mPciPlatformProtocol2 != NULL) {
    Status = mPciPlatformProtocol2->GetPciRom (
                                      (EFI_PCI_PLATFORM_PROTOCOL*)mPciPlatformProtocol2,
                                      PciIoDevice->Handle,
                                      &PlatformOpRomBuffer,
                                      &PlatformOpRomSize
                                      );
  } else if (mPciOverrideProtocol2 != NULL) {
    Status = mPciOverrideProtocol2->GetPciRom (
                                      (EFI_PCI_PLATFORM_PROTOCOL*)mPciOverrideProtocol2,
                                      PciIoDevice->Handle,
                                      &PlatformOpRomBuffer,
                                      &PlatformOpRomSize
                                      );
  } else {
    if (mPciPlatformProtocol != NULL) {
      Status = mPciPlatformProtocol->GetPciRom (
                                      mPciPlatformProtocol,
                                      PciIoDevice->Handle,
                                      &PlatformOpRomBuffer,
                                      &PlatformOpRomSize
                                      );
    } else if (mPciOverrideProtocol != NULL) {
      Status = mPciOverrideProtocol->GetPciRom (
                                        mPciOverrideProtocol,
                                        PciIoDevice->Handle,
                                        &PlatformOpRomBuffer,
                                        &PlatformOpRomSize
                                        );
    } else {
      //
      // return PCI Platform Protocol not found
      //
      return EFI_NOT_FOUND;
    }
  }

  if (!EFI_ERROR (Status)) {
    PciIoDevice->EmbeddedRom    = FALSE;
    PciIoDevice->RomSize        = (UINT32)PlatformOpRomSize;
    PciIoDevice->PciIo.RomSize  = PlatformOpRomSize;
    PciIoDevice->PciIo.RomImage = PlatformOpRomBuffer;
  }
  return Status;
}

/**
  Helper routine to indicate whether the given PCI device specific policy value
  dictates to override the Max_Payload_Size to a particular value, or set as per
  device capability.

  @param  MPS     Input device-specific policy should be in terms of type
                  EFI_PCI_CONF_MAX_PAYLOAD_SIZE

  @retval TRUE    Setup Max_Payload_Size as per device capability
          FALSE   override as per device-specific platform policy
**/
BOOLEAN
SetupMpsAsPerDeviceCapability (
  IN  UINT8                   MPS
)
{
  if ( MPS == EFI_PCI_CONF_MAX_PAYLOAD_SIZE_AUTO) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Helper routine to indicate whether the given PCI device specific policy value
  dictates to override the Max_Read_Req_Size to a particular value, or set as per
  device capability.

  @param  MRRS    Input device-specific policy should be in terms of type
                  EFI_PCI_CONF_MAX_READ_REQ_SIZE

  @retval TRUE    Setup Max_Read_Req_Size as per device capability
          FALSE   override as per device-specific platform policy
**/
BOOLEAN
SetupMrrsAsPerDeviceCapability (
  IN  UINT8                   MRRS
)
{
  if ( MRRS == EFI_PCI_CONF_MAX_READ_REQ_SIZE_AUTO) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Routine to translate the given device-specific platform policy from type
  EFI_PCI_CONF_MAX_PAYLOAD_SIZE to HW-specific value, as per PCI Base Specification
  Revision 4.0; for the PCI feature Max_Payload_Size.

  @param  MPS     Input device-specific policy should be in terms of type
                  EFI_PCI_CONF_MAX_PAYLOAD_SIZE

  @retval         Range values for the Max_Payload_Size as defined in the PCI
                  Base Specification 4.0
**/
UINT8
TranslateMpsSetupValueToPci (
  IN  UINT8                   MPS
)
{
  switch (MPS) {
    case EFI_PCI_CONF_MAX_PAYLOAD_SIZE_128B:
      return PCIE_MAX_PAYLOAD_SIZE_128B;
    case EFI_PCI_CONF_MAX_PAYLOAD_SIZE_256B:
      return PCIE_MAX_PAYLOAD_SIZE_256B;
    case EFI_PCI_CONF_MAX_PAYLOAD_SIZE_512B:
      return PCIE_MAX_PAYLOAD_SIZE_512B;
    case EFI_PCI_CONF_MAX_PAYLOAD_SIZE_1024B:
      return PCIE_MAX_PAYLOAD_SIZE_1024B;
    case EFI_PCI_CONF_MAX_PAYLOAD_SIZE_2048B:
      return PCIE_MAX_PAYLOAD_SIZE_2048B;
    case EFI_PCI_CONF_MAX_PAYLOAD_SIZE_4096B:
      return PCIE_MAX_PAYLOAD_SIZE_4096B;
    default:
      return PCIE_MAX_PAYLOAD_SIZE_128B;
  }
}

/**
  Routine to translate the given device-specific platform policy from type
  EFI_PCI_CONF_MAX_READ_REQ_SIZE to HW-specific value, as per PCI Base Specification
  Revision 4.0; for the PCI feature Max_Read_Req_Size.

  @param  MRRS    Input device-specific policy should be in terms of type
                  EFI_PCI_CONF_MAX_READ_REQ_SIZE

  @retval         Range values for the Max_Read_Req_Size as defined in the PCI
                  Base Specification 4.0
**/
UINT8
TranslateMrrsSetupValueToPci (
  IN  UINT8                   MRRS
)
{
  switch (MRRS) {
    case EFI_PCI_CONF_MAX_READ_REQ_SIZE_128B:
      return PCIE_MAX_READ_REQ_SIZE_128B;
    case EFI_PCI_CONF_MAX_READ_REQ_SIZE_256B:
      return PCIE_MAX_READ_REQ_SIZE_256B;
    case EFI_PCI_CONF_MAX_READ_REQ_SIZE_512B:
      return PCIE_MAX_READ_REQ_SIZE_512B;
    case EFI_PCI_CONF_MAX_READ_REQ_SIZE_1024B:
      return PCIE_MAX_READ_REQ_SIZE_1024B;
    case EFI_PCI_CONF_MAX_READ_REQ_SIZE_2048B:
      return PCIE_MAX_READ_REQ_SIZE_2048B;
    case EFI_PCI_CONF_MAX_READ_REQ_SIZE_4096B:
      return PCIE_MAX_READ_REQ_SIZE_4096B;
    default:
      return PCIE_MAX_READ_REQ_SIZE_128B;
  }
}

/**
  Generic routine to setup the PCI features as per its predetermined defaults.
**/
VOID
SetupDefaultsDevicePlatformPolicy (
  IN  PCI_IO_DEVICE               *PciDevice
  )
{
  PciDevice->SetupMPS = EFI_PCI_CONF_MAX_PAYLOAD_SIZE_AUTO;
  PciDevice->SetupMRRS = EFI_PCI_CONF_MAX_READ_REQ_SIZE_AUTO;
}

/**
  Intermediate routine to either get the PCI device specific platform policies
  through the PCI Platform Protocol, or its alias the PCI Override Protocol.

  @param  PciIoDevice         A pointer to PCI_IO_DEVICE
  @param  PciPlatformProtocol A pointer to EFI_PCI_PLATFORM_PROTOCOL2

  @retval EFI_STATUS          The direct status from the PCI Platform Protocol
  @retval EFI_SUCCESS         if on returning predetermined PCI features defaults,
                              for the case when protocol returns as EFI_UNSUPPORTED
                              to indicate PCI device exist and it has no platform
                              policy defined.
**/
EFI_STATUS
GetPciDevicePlatformPolicyEx (
  IN  PCI_IO_DEVICE               *PciIoDevice,
  IN  EFI_PCI_PLATFORM_PROTOCOL2  *PciPlatformProtocol
  )
{
  EFI_PCI_PLATFORM_EXTENDED_POLICY  PciPlatformExtendedPolicy;
  EFI_STATUS                        Status;

  ZeroMem ( &PciPlatformExtendedPolicy, sizeof (EFI_PCI_PLATFORM_EXTENDED_POLICY));
  Status = PciPlatformProtocol->GetDevicePolicy (
                                  PciPlatformProtocol,
                                  PciIoDevice->Handle,
                                  &PciPlatformExtendedPolicy
                                  );
  //
  // platform chipset policies are returned for this PCI device
  //
  if (!EFI_ERROR(Status)) {
    PciIoDevice->SetupMPS = PciPlatformExtendedPolicy.DeviceCtlMPS;
    PciIoDevice->SetupMRRS = PciPlatformExtendedPolicy.DeviceCtlMRRS;
  }
  //
  // platform chipset policies are not provided for this PCI device
  //
  if (EFI_ERROR(Status) == EFI_UNSUPPORTED) {
    //
    // let the enumeration happen as per the PCI standard way
    //
    SetupDefaultsDevicePlatformPolicy ( PciIoDevice);
    return EFI_SUCCESS;
  }

  return Status;
}

/**
  Gets the PCI device-specific platform policy from the PCI Platform Protocol.
  If no PCI Platform protocol is published than setup the PCI feature to predetermined
  defaults, in order to align all the PCI devices in the PCI hierarchy, as applicable.

  @param  PciDevice     A pointer to PCI_IO_DEVICE

  @retval EFI_STATUS    The direct status from the PCI Platform Protocol
  @retval EFI_SUCCESS   On return of predetermined PCI features defaults, for
                        the case when protocol returns as EFI_UNSUPPORTED to
                        indicate PCI device exist and it has no platform policy
                        defined. Also, on returns when no PCI Platform Protocol
                        exist.
**/
EFI_STATUS
GetPciDevicePlatformPolicy (
  IN PCI_IO_DEVICE          *PciDevice
  )
{
  if ( mPciPlatformProtocol2 != NULL) {
    return GetPciDevicePlatformPolicyEx ( PciDevice, mPciPlatformProtocol2);
  } else if (mPciOverrideProtocol2 != NULL) {
    return GetPciDevicePlatformPolicyEx ( PciDevice, mPciOverrideProtocol2);
  } else {
    //
    // new PCI Platform Protocol 2 is not installed; let the enumeration happen
    // as per PCI standard way
    //
    SetupDefaultsDevicePlatformPolicy ( PciDevice);
    return EFI_SUCCESS;
  }
}
