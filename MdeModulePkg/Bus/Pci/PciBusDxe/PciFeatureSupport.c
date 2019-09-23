/** @file
  PCI standard feature support functions implementation for PCI Bus module..

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PciFeatureSupport.h"

/**
  A gobal pointer to PRIMARY_ROOT_PORT_NODE buffer to track all the primary physical
  PCI Root Ports (PCI Controllers) for a given PCI Root Bridge instance while
  enumerating to configure the PCI features
**/
PRIMARY_ROOT_PORT_NODE                      *mPrimaryRootPortList;

/**
  A gobal pointer to OTHER_PCI_FEATURES_CONFIGURATION_TABLE buffer all the PCI
  Feature configuration Table nodes to pair against each of the PRIMARY_ROOT_PORT_NODE
  buffer nodes. Each node of these is used to align all the PCI devices originating
  from the PCI Root Port devices of a PCI Root Bridge instance
**/
OTHER_PCI_FEATURES_CONFIGURATION_TABLE      *mPciFeaturesConfigurationTableInstances;

/**
  A global pointer to PCI_FEATURE_CONFIGURATION_COMPLETION_LIST, which stores all
  the PCI Root Bridge instances that are enumerated for the other PCI features,
  like MaxPayloadSize & MaxReadReqSize; during the the Start() interface of the
  driver binding protocol. The records pointed by this pointer would be destroyed
  when the DXE core invokes the Stop() interface.
**/
PCI_FEATURE_CONFIGURATION_COMPLETION_LIST   *mPciFeaturesConfigurationCompletionList = NULL;

/**
  Main routine to indicate platform selection of any of the other PCI features
  to be configured by this driver

  @retval TRUE    platform has selected the other PCI features to be configured
          FALSE   platform has not selected any of the other PCI features
**/
BOOLEAN
CheckOtherPciFeaturesPcd (
  )
{
  return PcdGet32 ( PcdOtherPciFeatures) ? TRUE : FALSE;
}

/**
  Main routine to indicate whether the platform has selected the Max_Payload_Size
  PCI feature to be configured by this driver

  @retval TRUE    platform has selected the Max_Payload_Size to be configured
          FALSE   platform has not selected this feature
**/
BOOLEAN
SetupMaxPayloadSize (
  )
{
  return (PcdGet32 ( PcdOtherPciFeatures) & PCI_FEATURE_SUPPORT_FLAG_MPS) ? TRUE : FALSE;
}

/**
  Main routine to indicate whether the platform has selected the Max_Read_Req_Size
  PCI feature to be configured by this driver

  @retval TRUE    platform has selected the Max_Read_Req_Size to be configured
          FALSE   platform has not selected this feature
**/
BOOLEAN
SetupMaxReadReqSize (
  )
{
  return (PcdGet32 ( PcdOtherPciFeatures) & PCI_FEATURE_SUPPORT_FLAG_MRRS) ? TRUE : FALSE;
}

/**
  Helper routine which determines whether the given PCI Root Bridge instance
  record already exist. This routine shall help avoid duplicate record creation
  in case of re-enumeration of PCI configuation features.

  @param  RootBridge              A pointer to the PCI_IO_DEVICE for the Root Bridge
  @param  PciFeatureConfigRecord  A pointer to a pointer for type
                                  PCI_FEATURE_CONFIGURATION_COMPLETION_LIST
                                  record, Use to return the specific record.

  @retval TRUE                    Record already exist
          FALSE                   Record does not exist for the given PCI Root Bridge
**/
BOOLEAN
CheckPciFeatureConfigurationRecordExist (
  IN  PCI_IO_DEVICE                             *RootBridge,
  OUT PCI_FEATURE_CONFIGURATION_COMPLETION_LIST **PciFeatureConfigRecord
  )
{
  LIST_ENTRY                                  *Link;
  PCI_FEATURE_CONFIGURATION_COMPLETION_LIST   *Temp;

  if ( mPciFeaturesConfigurationCompletionList) {
    Link = &mPciFeaturesConfigurationCompletionList->RootBridgeLink;

    do {
      Temp = PCI_FEATURE_CONFIGURATION_COMPLETION_LIST_FROM_LINK (Link);
      if ( Temp->RootBridgeHandle == RootBridge->Handle) {
        *PciFeatureConfigRecord = Temp;
        return TRUE;
      }
      Link = Link->ForwardLink;
    } while (Link != &mPciFeaturesConfigurationCompletionList->RootBridgeLink);
  }
  //
  // not found on the PCI feature configuration completion list
  //
  *PciFeatureConfigRecord = NULL;
  return FALSE;
}

/**
  This routine is primarily to avoid multiple configuration of PCI features
  to the same PCI Root Bridge due to EDK2 core's ConnectController calls on
  all the EFI handles. This routine also provide re-enumeration of the PCI
  features on the same PCI Root Bridge based on the policy of ReEnumeratePciFeatureConfiguration
  of the PCI_FEATURE_CONFIGURATION_COMPLETION_LIST.

  @param  RootBridge              A pointer to the PCI_IO_DEVICE for the Root Bridge

  @retval TRUE                    PCI Feature configuration required for the PCI
                                  Root Bridge
          FALSE                   PCI Feature configuration is not required to be
                                  re-enumerated for the PCI Root Bridge
**/
BOOLEAN
CheckPciFeaturesConfigurationRequired (
  IN PCI_IO_DEVICE          *RootBridge
  )
{
  LIST_ENTRY                                  *Link;
  PCI_FEATURE_CONFIGURATION_COMPLETION_LIST   *Temp;

  if ( mPciFeaturesConfigurationCompletionList) {
    Link = &mPciFeaturesConfigurationCompletionList->RootBridgeLink;

    do {
      Temp = PCI_FEATURE_CONFIGURATION_COMPLETION_LIST_FROM_LINK (Link);
      if ( Temp->RootBridgeHandle == RootBridge->Handle) {
        return Temp->ReEnumeratePciFeatureConfiguration;
      }
      Link = Link->ForwardLink;
    } while (Link != &mPciFeaturesConfigurationCompletionList->RootBridgeLink);
  }
  //
  // not found on the PCI feature configuration completion list, return as required
  //
  return TRUE;
}

/**
  This routine finds the duplicate record if exist and assigns the re-enumeration
  requirement flag, as passed as input. It creates new record for the PCI Root
  Bridge and appends the list after updating its re-enumeration flag.

  @param  RootBridge            A pointer to PCI_IO_DEVICE of the Root Bridge
  @param  ReEnumerationRequired A BOOLEAN for recording the re-enumeration requirement

  @retval EFI_SUCCESS           new record inserted into the list or updated the
                                existing record
          EFI_INVALID_PARAMETER Unexpected error as CheckPciFeatureConfigurationRecordExist
                                reports as record exist but does not return its pointer
          EFI_OUT_OF_RESOURCES  Not able to create PCI features configuratin complete
                                record for the RootBridge
**/
EFI_STATUS
AddRootBridgeInPciFeaturesConfigCompletionList (
  IN PCI_IO_DEVICE          *RootBridge,
  IN BOOLEAN                ReEnumerationRequired
  )
{
  PCI_FEATURE_CONFIGURATION_COMPLETION_LIST   *Temp;

  if ( CheckPciFeatureConfigurationRecordExist ( RootBridge, &Temp)) {
    //
    // this PCI Root Bridge record already exist; it may have been re-enumerated
    // hence just update its enumeration required flag again to exit
    //
    if ( Temp) {
      Temp->ReEnumeratePciFeatureConfiguration  = ReEnumerationRequired;
      return EFI_SUCCESS;
    } else {
      //
      // PCI feature configuration complete record reported as exist and no
      // record pointer returned
      //
      return EFI_INVALID_PARAMETER;
    }

  } else {

    Temp = AllocateZeroPool ( sizeof ( PCI_FEATURE_CONFIGURATION_COMPLETION_LIST));
    if ( !Temp) {
      return EFI_OUT_OF_RESOURCES;
    }
    Temp->Signature                           = PCI_FEATURE_CONFIGURATION_SIGNATURE;
    Temp->RootBridgeHandle                    = RootBridge->Handle;
    Temp->ReEnumeratePciFeatureConfiguration  = ReEnumerationRequired;
    if ( mPciFeaturesConfigurationCompletionList) {
      InsertTailList ( &mPciFeaturesConfigurationCompletionList->RootBridgeLink,
                       &Temp->RootBridgeLink);
    } else {
      //
      // init the very first node of the Root Bridge
      //
      mPciFeaturesConfigurationCompletionList = Temp;
      InitializeListHead ( &mPciFeaturesConfigurationCompletionList->RootBridgeLink);
    }
  }
  return EFI_SUCCESS;
}

/**
  Free up memory alloted for the primary physical PCI Root ports of the PCI Root
  Bridge instance. Free up all the nodes of type PRIMARY_ROOT_PORT_NODE.
**/
VOID
DestroyPrimaryRootPortNodes ()
{
  LIST_ENTRY                *Link;
  PRIMARY_ROOT_PORT_NODE    *Temp;

  if ( mPrimaryRootPortList) {
    Link = &mPrimaryRootPortList->NeighborRootPort;

    if ( IsListEmpty ( Link)) {
      FreePool ( mPrimaryRootPortList);
    } else {
      do {
        if ( Link->ForwardLink != &mPrimaryRootPortList->NeighborRootPort) {
          Link = Link->ForwardLink;
        }
        Temp = PRIMARY_ROOT_PORT_NODE_FROM_LINK ( Link);
        Link = RemoveEntryList ( Link);
        FreePool ( Temp);
      } while ( !IsListEmpty ( Link));
      FreePool ( mPrimaryRootPortList);
    }
    mPrimaryRootPortList = NULL;
  }
}

/**
  Free up the memory allocated for temporarily maintaining the PCI feature
  configuration table for all the nodes of the primary PCI Root port.
  Free up memory alloted for OTHER_PCI_FEATURES_CONFIGURATION_TABLE.
**/
VOID
ErasePciFeaturesConfigurationTable (
  )
{
  if ( mPciFeaturesConfigurationTableInstances) {
    FreePool ( mPciFeaturesConfigurationTableInstances);
  }
  mPciFeaturesConfigurationTableInstances = NULL;
}

/**
  Routine meant for initializing any global variables used. It primarily cleans
  up the internal data structure memory allocated for the previous PCI Root Bridge
  instance. This should be the first routine to call for any virtual PCI Root
  Bridge instance.
**/
VOID
SetupPciFeaturesConfigurationDefaults ()
{
  //
  // delete the primary root port list
  //
  if (mPrimaryRootPortList) {
    DestroyPrimaryRootPortNodes ();
  }

  if ( mPciFeaturesConfigurationTableInstances) {
    ErasePciFeaturesConfigurationTable ();
  }
}

/**
  Helper routine to determine whether the PCI device is a physical root port
  recorded in the list.

  @param  PciDevice                       A pointer to the PCI_IO_DEVICE.

  @retval TRUE                            The PCI device instance is a primary
                                          primary physical PCI Root Port
          FALSE                           Not a primary physical PCI Root port
**/
BOOLEAN
CheckRootBridgePrimaryPort (
  IN PCI_IO_DEVICE          *PciDevice
  )
{
  LIST_ENTRY                *Link;
  PRIMARY_ROOT_PORT_NODE    *Temp;

  if ( !mPrimaryRootPortList) {
    return FALSE;
  }
  Link = &mPrimaryRootPortList->NeighborRootPort;
  do {
    Temp = PRIMARY_ROOT_PORT_NODE_FROM_LINK ( Link);
    if ( Temp->RootBridgeHandle == PciDevice->Parent->Handle
        && Temp->RootPortHandle == PciDevice->Handle) {
      //
      // the given PCI device is the primary root port of the Root Bridge controller
      //
      return TRUE;
    }
    Link = Link->ForwardLink;
  } while ( Link != &mPrimaryRootPortList->NeighborRootPort);
  //
  // the given PCI device is not the primary root port of the Bridge controller
  //
  return FALSE;
}

/**
  Main routine to determine the child PCI devices of a physical PCI bridge device
  and group them under a common internal PCI features Configuration table.

  @param  PciDevice                       A pointer to the PCI_IO_DEVICE.
  @param  PciFeaturesConfigTable          A pointer to a pointer to the
                                          OTHER_PCI_FEATURES_CONFIGURATION_TABLE.
                                          Returns NULL in case of RCiEP or the PCI
                                          device does match with any of the physical
                                          Root ports, or it does not belong to any
                                          Root port's PCU bus range (not a child)

  @retval EFI_SUCCESS                     able to determine the PCI feature
                                          configuration table. For RCiEP since
                                          since it is not prepared.
          EFI_NOT_FOUND                   the PCI feature configuration table does
                                          not exist as the PCI physical Bridge device
                                          is not found for this device's parent
                                          Root Bridge instance
**/
EFI_STATUS
GetPciFeaturesConfigurationTable (
  IN  PCI_IO_DEVICE                           *PciDevice,
  OUT OTHER_PCI_FEATURES_CONFIGURATION_TABLE  **PciFeaturesConfigTable
  )
{
  LIST_ENTRY                *Link;
  PRIMARY_ROOT_PORT_NODE    *Temp;

  if ( !mPrimaryRootPortList) {
    *PciFeaturesConfigTable = NULL;
    return EFI_NOT_FOUND;
  }

  //
  // The PCI features configuration table is not built for RCiEP, return NULL
  //
  if ( PciDevice->PciExpStruct.Capability.Bits.DevicePortType == \
        PCIE_DEVICE_PORT_TYPE_ROOT_COMPLEX_INTEGRATED_ENDPOINT) {
    *PciFeaturesConfigTable = NULL;
    return EFI_SUCCESS;
  }

  Link = &mPrimaryRootPortList->NeighborRootPort;
  do {
    Temp = PRIMARY_ROOT_PORT_NODE_FROM_LINK ( Link);
    if ( Temp->RootBridgeHandle == PciDevice->Parent->Handle
        && Temp->RootPortHandle == PciDevice->Handle) {
      //
      // the given PCI device is the primary root port of the Root Bridge controller
      //
      *PciFeaturesConfigTable = Temp->OtherPciFeaturesConfigurationTable;
      return EFI_SUCCESS;
    } else {
      //
      // check this PCI device belongs to the primary root port of the root bridge
      //
      if ( PciDevice->BusNumber >= Temp->SecondaryBusStart
          && PciDevice->BusNumber <= Temp->SecondaryBusEnd) {
        *PciFeaturesConfigTable = Temp->OtherPciFeaturesConfigurationTable;
        return EFI_SUCCESS;
      }
    }
    Link = Link->ForwardLink;
  } while ( Link != &mPrimaryRootPortList->NeighborRootPort);
  //
  // the PCI device must be RCiEP, does not belong to any primary root port
  //
  *PciFeaturesConfigTable = NULL;
  return EFI_SUCCESS;
}

/**
  The helper routine to retrieve the PCI bus numbers from the PCI Bridge or Root
  port device. Assumes the input PCI device has the PCI Type 1 configuration header.

  @param  PciDevice                       A pointer to the PCI_IO_DEVICE.
  @param  PrimaryBusNumber                A pointer to return the PCI Priamry
                                          Bus number.
  @param  SecondaryBusNumber              A pointer to return the PCI Secondary
                                          Bus number.
  @param  SubordinateBusNumber            A pointer to return the PCI Subordinate
                                          Bus number.

  @retval EFI_SUCCESS           The data was read from the PCI device.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI device.
  @retval EFI_INVALID_PARAMETER input parameters provided to the read operation were invalid.
**/
EFI_STATUS
GetPciRootPortBusAssigned (
  IN  PCI_IO_DEVICE                           *PciDevice,
  OUT UINT8                                   *PrimaryBusNumber,
  OUT UINT8                                   *SecondaryBusNumber,
  OUT UINT8                                   *SubordinateBusNumber
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      RootPortBusAssigned;

  Status = PciDevice->PciIo.Pci.Read (
                                  &PciDevice->PciIo,
                                  EfiPciIoWidthUint32,
                                  PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
                                  1,
                                  &RootPortBusAssigned
                                );
  if ( !EFI_ERROR(Status)) {
    if ( PrimaryBusNumber) {
      *PrimaryBusNumber = (UINT8) (0xFF & RootPortBusAssigned);
    }
    if ( SecondaryBusNumber) {
      *SecondaryBusNumber = (UINT8)(0xFF & (RootPortBusAssigned >> 8));
    }
    if ( SubordinateBusNumber) {
      *SubordinateBusNumber = (UINT8)(0xFF & (RootPortBusAssigned >> 16));
    }
  }
  return Status;
}

/**
  This routine determines the existance of the child PCI device for the given
  PCI Root / Bridge Port device. Always assumes the input PCI device is the bridge
  or PCI-PCI Bridge device. This routine should not be used with PCI endpoint device.

  @param  PciDevice                       A pointer to the PCI_IO_DEVICE.

  @retval TRUE                            child device exist
          FALSE                           no child device
**/
BOOLEAN
IsPciRootPortEmpty (
  IN  PCI_IO_DEVICE                           *PciDevice
  )
{
  UINT8                                       SecBus,
                                              SubBus;
  EFI_STATUS                                  Status;
  LIST_ENTRY                                  *Link;
  PCI_IO_DEVICE                               *NextPciDevice;

  //
  // check secondary & suboridinate bus numbers for its endpoint device
  // existance
  //
  Status = GetPciRootPortBusAssigned ( PciDevice, NULL, &SecBus, &SubBus);
  if ( !EFI_ERROR( Status)) {
    Link = PciDevice->ChildList.ForwardLink;
    if ( IsListEmpty ( Link)) {
      //
      // return as PCI Root port empty
      //
      DEBUG (( DEBUG_INFO, "RP empty,"));
      return TRUE;
    }
    do {
      NextPciDevice = PCI_IO_DEVICE_FROM_LINK ( Link);
      DEBUG (( DEBUG_INFO, "dev@%x", NextPciDevice->BusNumber));

      if ( NextPciDevice->BusNumber >= SecBus
          && NextPciDevice->BusNumber <= SubBus) {

        return FALSE;
      }

      Link = Link->ForwardLink;
    } while ( Link != &PciDevice->ChildList);
  } else {
    SecBus = SubBus = 0;
    DEBUG (( DEBUG_ERROR, "unable to retrieve root port's bus range assigned!!!"));
  }

  //
  // return as PCI Root port empty
  //
  return TRUE;
}

/**
  The main routine which process the PCI feature Max_Payload_Size as per the
  device-specific platform policy, as well as in complaince with the PCI Base
  specification Revision 4, that aligns the value for the entire PCI heirarchy
  starting from its physical PCI Root port / Bridge device.

  @param PciDevice                      A pointer to the PCI_IO_DEVICE.
  @param PciConfigPhase                 for the PCI feature configuration phases:
                                        PciFeatureGetDevicePolicy & PciFeatureSetupPhase
  @param PciFeaturesConfigurationTable  pointer to OTHER_PCI_FEATURES_CONFIGURATION_TABLE

  @retval EFI_SUCCESS                   processing of PCI feature Max_Payload_Size
                                        is successful.
**/
EFI_STATUS
ProcessMaxPayloadSize (
  IN  PCI_IO_DEVICE                           *PciDevice,
  IN  PCI_FEATURE_CONFIGURATION_PHASE         PciConfigPhase,
  IN  OTHER_PCI_FEATURES_CONFIGURATION_TABLE  *PciFeaturesConfigurationTable
  )
{
  PCI_REG_PCIE_DEVICE_CAPABILITY          PciDeviceCap;
  UINT8                                   MpsValue;


  PciDeviceCap.Uint32 = PciDevice->PciExpStruct.DeviceCapability.Uint32;

  if ( PciConfigPhase == PciFeatureGetDevicePolicy) {
    if ( SetupMpsAsPerDeviceCapability ( PciDevice->SetupMPS))
    {
      MpsValue = (UINT8)PciDeviceCap.Bits.MaxPayloadSize;
      //
      // no change to PCI Root ports without any endpoint device
      //
      if ( IS_PCI_BRIDGE ( &PciDevice->Pci) && PciDeviceCap.Bits.MaxPayloadSize) {
        if ( IsPciRootPortEmpty ( PciDevice)) {
          MpsValue = PCIE_MAX_PAYLOAD_SIZE_128B;
          DEBUG (( DEBUG_INFO, "(reset RP MPS to min.)"));
        }
      }
    } else {
      MpsValue = TranslateMpsSetupValueToPci ( PciDevice->SetupMPS);
    }
    //
    // discard device policy override request if greater than PCI device capability
    //
    PciDevice->SetupMPS = MIN( (UINT8)PciDeviceCap.Bits.MaxPayloadSize, MpsValue);
  }

  //
  // align the MPS of the tree to the HCF with this device
  //
  if ( PciFeaturesConfigurationTable) {
    MpsValue = PciFeaturesConfigurationTable->Max_Payload_Size;

    MpsValue = MIN ( PciDevice->SetupMPS, MpsValue);
    PciDevice->SetupMPS = MIN ( PciDevice->SetupMPS, MpsValue);

    if ( MpsValue != PciFeaturesConfigurationTable->Max_Payload_Size) {
      DEBUG (( DEBUG_INFO, "reset MPS of the tree to %d,", MpsValue));
      PciFeaturesConfigurationTable->Max_Payload_Size = MpsValue;
    }
  }

  DEBUG (( DEBUG_INFO,
      "Max_Payload_Size: %d [DevCap:%d],",
      PciDevice->SetupMPS, PciDeviceCap.Bits.MaxPayloadSize
  ));
  return EFI_SUCCESS;
}

/**
  The main routine which process the PCI feature Max_Read_Req_Size as per the
  device-specific platform policy, as well as in complaince with the PCI Base
  specification Revision 4, that aligns the value for the entire PCI heirarchy
  starting from its physical PCI Root port / Bridge device.

  @param PciDevice                      A pointer to the PCI_IO_DEVICE.
  @param PciConfigPhase                 for the PCI feature configuration phases:
                                        PciFeatureGetDevicePolicy & PciFeatureSetupPhase
  @param PciFeaturesConfigurationTable  pointer to OTHER_PCI_FEATURES_CONFIGURATION_TABLE

  @retval EFI_SUCCESS                   processing of PCI feature Max_Read_Req_Size
                                        is successful.
**/
EFI_STATUS
ProcessMaxReadReqSize (
  IN  PCI_IO_DEVICE                           *PciDevice,
  IN  PCI_FEATURE_CONFIGURATION_PHASE         PciConfigPhase,
  IN  OTHER_PCI_FEATURES_CONFIGURATION_TABLE  *PciFeaturesConfigurationTable
  )
{
  PCI_REG_PCIE_DEVICE_CAPABILITY  PciDeviceCap;
  UINT8                           MrrsValue;

  PciDeviceCap.Uint32 = PciDevice->PciExpStruct.DeviceCapability.Uint32;

  if ( PciConfigPhase == PciFeatureGetDevicePolicy) {
    if ( SetupMrrsAsPerDeviceCapability ( PciDevice->SetupMRRS))
    {
      //
      // The maximum read request size is not the data packet size of the TLP,
      // but the memory read request size, and set to the function as a requestor
      // to not exceed this limit.
      // However, for the PCI device capable of isochronous traffic; this memory read
      // request size should not extend beyond the Max_Payload_Size. Thus, in case if
      // device policy return by platform indicates to set as per device capability
      // than set as per Max_Payload_Size configuration value
      //
      if ( SetupMaxPayloadSize()) {
        MrrsValue = PciDevice->SetupMPS;
      } else {
        //
        // in case this driver is not required to configure the Max_Payload_Size
        // than consider programming HCF of the device capability's Max_Payload_Size
        // in this PCI hierarchy; thus making this an implementation specific feature
        // which the platform should avoid. For better results, the platform should
        // make both the Max_Payload_Size & Max_Read_Request_Size to be configured
        // by this driver
        //
        MrrsValue = (UINT8)PciDeviceCap.Bits.MaxPayloadSize;
      }
    } else {
      //
      // override as per platform based device policy
      //
      MrrsValue = TranslateMrrsSetupValueToPci ( PciDevice->SetupMRRS);
      //
      // align this device's Max_Read_Request_Size value to the entire PCI tree
      //
      if ( PciFeaturesConfigurationTable) {
        if ( !PciFeaturesConfigurationTable->Lock_Max_Read_Request_Size) {
          PciFeaturesConfigurationTable->Lock_Max_Read_Request_Size = TRUE;
          PciFeaturesConfigurationTable->Max_Read_Request_Size = MrrsValue;
        } else {
          //
          // in case of another user enforced value of MRRS within the same tree,
          // pick the smallest between the locked value and this value; to set
          // across entire PCI tree nodes
          //
          MrrsValue = MIN (
                        MrrsValue,
                        PciFeaturesConfigurationTable->Max_Read_Request_Size
                        );
          PciFeaturesConfigurationTable->Max_Read_Request_Size = MrrsValue;
        }
      }
    }
    //
    // align this device's Max_Read_Request_Size to derived configuration value
    //
    PciDevice->SetupMRRS = MrrsValue;

  }

  //
  // align the Max_Read_Request_Size of the PCI tree based on 3 conditions:
  // first, if user defines MRRS for any one PCI device in the tree than align
  // all the devices in the PCI tree.
  // second, if user override is not define for this PCI tree than setup the MRRS
  // based on MPS value of the tree to meet the criteria for the isochronous
  // traffic.
  // third, if no user override, or platform firmware policy has not selected
  // this PCI bus driver to configure the MPS; than configure the MRRS to a
  // highest common value of PCI device capability for the MPS found among all
  // the PCI devices in this tree
  //
  if ( PciFeaturesConfigurationTable) {
    if ( PciFeaturesConfigurationTable->Lock_Max_Read_Request_Size) {
      PciDevice->SetupMRRS = PciFeaturesConfigurationTable->Max_Read_Request_Size;
    } else {
      if ( SetupMaxPayloadSize()) {
        PciDevice->SetupMRRS = PciDevice->SetupMPS;
      } else {
        PciDevice->SetupMRRS = MIN (
                                PciDevice->SetupMRRS,
                                PciFeaturesConfigurationTable->Max_Read_Request_Size
                                );
      }
      PciFeaturesConfigurationTable->Max_Read_Request_Size = PciDevice->SetupMRRS;
    }
  }
  DEBUG (( DEBUG_INFO, "Max_Read_Request_Size: %d\n", PciDevice->SetupMRRS));

  return EFI_SUCCESS;
}

/**
  Overrides the PCI Device Control register MaxPayloadSize register field; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI device.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
OverrideMaxPayloadSize (
  IN PCI_IO_DEVICE          *PciDevice
  )
{
  PCI_REG_PCIE_DEVICE_CONTROL PcieDev;
  UINT32                      Offset;
  EFI_STATUS                  Status;

  PcieDev.Uint16 = 0;
  Offset = PciDevice->PciExpressCapabilityOffset +
               OFFSET_OF (PCI_CAPABILITY_PCIEXP, DeviceControl);
  Status = PciDevice->PciIo.Pci.Read (
                                  &PciDevice->PciIo,
                                  EfiPciIoWidthUint16,
                                  Offset,
                                  1,
                                  &PcieDev.Uint16
                                );
  if ( EFI_ERROR(Status)){
    DEBUG (( DEBUG_ERROR, "Unexpected DeviceControl register (0x%x) read error!",
        Offset
    ));
    return Status;
  }
  if ( PcieDev.Bits.MaxPayloadSize != PciDevice->SetupMPS) {
    PcieDev.Bits.MaxPayloadSize = PciDevice->SetupMPS;
    DEBUG (( DEBUG_INFO, "Max_Payload_Size=%d,", PciDevice->SetupMPS));

    Status = PciDevice->PciIo.Pci.Write (
                                    &PciDevice->PciIo,
                                    EfiPciIoWidthUint16,
                                    Offset,
                                    1,
                                    &PcieDev.Uint16
                                  );
    if ( !EFI_ERROR(Status)) {
      PciDevice->PciExpStruct.DeviceControl.Uint16 = PcieDev.Uint16;
    } else {
      DEBUG (( DEBUG_ERROR, "Unexpected DeviceControl register (0x%x) write error!",
          Offset
      ));
    }
  } else {
    DEBUG (( DEBUG_INFO, "No write of Max_Payload_Size=%d,", PciDevice->SetupMPS));
  }

  return Status;
}

/**
  Overrides the PCI Device Control register MaxPayloadSize register field; if
  the hardware value is different than the intended value.

  @param  PciDevice             A pointer to the PCI_IO_DEVICE instance.

  @retval EFI_SUCCESS           The data was read from or written to the PCI controller.
  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.

**/
EFI_STATUS
OverrideMaxReadReqSize (
  IN PCI_IO_DEVICE          *PciDevice
  )
{
  PCI_REG_PCIE_DEVICE_CONTROL PcieDev;
  UINT32                      Offset;
  EFI_STATUS                  Status;

  PcieDev.Uint16 = 0;
  Offset = PciDevice->PciExpressCapabilityOffset +
               OFFSET_OF (PCI_CAPABILITY_PCIEXP, DeviceControl);
  Status = PciDevice->PciIo.Pci.Read (
                                  &PciDevice->PciIo,
                                  EfiPciIoWidthUint16,
                                  Offset,
                                  1,
                                  &PcieDev.Uint16
                                );
  if ( EFI_ERROR(Status)){
    DEBUG (( DEBUG_ERROR, "Unexpected DeviceControl register (0x%x) read error!",
        Offset
    ));
    return Status;
  }
  if ( PcieDev.Bits.MaxReadRequestSize != PciDevice->SetupMRRS) {
    PcieDev.Bits.MaxReadRequestSize = PciDevice->SetupMRRS;
    DEBUG (( DEBUG_INFO, "Max_Read_Request_Size: %d\n", PciDevice->SetupMRRS));

    Status = PciDevice->PciIo.Pci.Write (
                                    &PciDevice->PciIo,
                                    EfiPciIoWidthUint16,
                                    Offset,
                                    1,
                                    &PcieDev.Uint16
                                  );
    if ( !EFI_ERROR(Status)) {
      PciDevice->PciExpStruct.DeviceControl.Uint16 = PcieDev.Uint16;
    } else {
      DEBUG (( DEBUG_ERROR, "Unexpected DeviceControl register (0x%x) write error!",
          Offset
      ));
    }
  } else {
    DEBUG (( DEBUG_INFO, "No write of Max_Read_Request_Size=%d\n", PciDevice->SetupMRRS));
  }

  return Status;
}

/**
  helper routine to dump the PCIe Device Port Type
**/
VOID
DumpDevicePortType (
  IN  UINT8   DevicePortType
  )
{
  switch ( DevicePortType){
    case PCIE_DEVICE_PORT_TYPE_PCIE_ENDPOINT:
      DEBUG (( DEBUG_INFO, "PCIe endpoint found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_LEGACY_PCIE_ENDPOINT:
      DEBUG (( DEBUG_INFO, "legacy PCI endpoint found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_ROOT_PORT:
      DEBUG (( DEBUG_INFO, "PCIe Root Port found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_UPSTREAM_PORT:
      DEBUG (( DEBUG_INFO, "PCI switch upstream port found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_DOWNSTREAM_PORT:
      DEBUG (( DEBUG_INFO, "PCI switch downstream port found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_PCIE_TO_PCI_BRIDGE:
      DEBUG (( DEBUG_INFO, "PCIe-PCI bridge found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_PCI_TO_PCIE_BRIDGE:
      DEBUG (( DEBUG_INFO, "PCI-PCIe bridge found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_ROOT_COMPLEX_INTEGRATED_ENDPOINT:
      DEBUG (( DEBUG_INFO, "RCiEP found\n"));
      break;
    case PCIE_DEVICE_PORT_TYPE_ROOT_COMPLEX_EVENT_COLLECTOR:
      DEBUG (( DEBUG_INFO, "RC Event Collector found\n"));
      break;
  }
}

/**
   Process each PCI device as per the pltaform and device-specific policy.

  @param RootBridge             A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           processing each PCI feature as per policy defined
                                was successful.
 **/
EFI_STATUS
SetupDevicePciFeatures (
  IN  PCI_IO_DEVICE                   *PciDevice,
  IN  PCI_FEATURE_CONFIGURATION_PHASE PciConfigPhase
  )
{
  EFI_STATUS                              Status;
  PCI_REG_PCIE_CAPABILITY                 PcieCap;
  OTHER_PCI_FEATURES_CONFIGURATION_TABLE  *OtherPciFeaturesConfigTable;

  PcieCap.Uint16 = PciDevice->PciExpStruct.Capability.Uint16;
  DumpDevicePortType ( (UINT8)PcieCap.Bits.DevicePortType);

  OtherPciFeaturesConfigTable = NULL;
  Status = GetPciFeaturesConfigurationTable ( PciDevice, &OtherPciFeaturesConfigTable);
  if ( EFI_ERROR( Status)) {
    DEBUG ((
        EFI_D_WARN, "No primary root port found in these root bridge nodes!\n"
    ));
  } else if ( !OtherPciFeaturesConfigTable) {
    DEBUG ((
        DEBUG_INFO, "No PCI features config. table for this device!\n"
    ));
  } else {
    DEBUG ((
        DEBUG_INFO, "using PCI features config. table ID: %d\n",
        OtherPciFeaturesConfigTable->ID
    ));
  }

  if ( PciConfigPhase == PciFeatureGetDevicePolicy) {
    Status = GetPciDevicePlatformPolicy ( PciDevice);
    if ( EFI_ERROR(Status)) {
      DEBUG ((
          DEBUG_ERROR, "Error in obtaining PCI device policy!!!\n"
      ));
    }
  }

  if ( SetupMaxPayloadSize ()) {
    Status = ProcessMaxPayloadSize (
              PciDevice,
              PciConfigPhase,
              OtherPciFeaturesConfigTable
              );
  }
  //
  // implementation specific rule:- the MRRS of any PCI device should be processed
  // only after the MPS is processed for that device
  //
  if ( SetupMaxReadReqSize ()) {
    Status = ProcessMaxReadReqSize (
              PciDevice,
              PciConfigPhase,
              OtherPciFeaturesConfigTable
              );
  }
  return Status;
}

/**
  Traverse all the nodes from the root bridge or PCI-PCI bridge instance, to
  configure the PCI features as per the device-specific platform policy, and
  as per the device capability, as applicable.

  @param RootBridge             A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           Traversing all the nodes of the root bridge
                                instances were successfull.
**/
EFI_STATUS
SetupPciFeatures (
  IN  PCI_IO_DEVICE                   *RootBridge,
  IN  PCI_FEATURE_CONFIGURATION_PHASE PciConfigPhase
  )
{
  EFI_STATUS           Status;
  LIST_ENTRY           *Link;
  PCI_IO_DEVICE        *Device;

  for ( Link = RootBridge->ChildList.ForwardLink
      ; Link != &RootBridge->ChildList
      ; Link = Link->ForwardLink
  ) {
    Device = PCI_IO_DEVICE_FROM_LINK (Link);
    if (IS_PCI_BRIDGE (&Device->Pci)) {
      DEBUG ((
          DEBUG_INFO, "::Bridge [%02x|%02x|%02x] -",
          Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
      ));
      if (Device->IsPciExp) {
        Status = SetupDevicePciFeatures ( Device, PciConfigPhase);
      } else {
        DEBUG (( DEBUG_INFO, "Not a PCIe capable device!\n"));
        //
        // PCI Bridge which does not have PCI Express Capability structure
        // cannot process this kind of PCI Bridge device
        //

      }

      SetupPciFeatures ( Device, PciConfigPhase);
    } else {
      DEBUG ((
          DEBUG_INFO, "::Device [%02x|%02x|%02x] -",
          Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
      ));
      if (Device->IsPciExp) {

        Status = SetupDevicePciFeatures ( Device, PciConfigPhase);
      } else {
        DEBUG (( DEBUG_INFO, "Not a PCIe capable device!\n"));
        //
        // PCI Device which does not have PCI Express Capability structure
        // cannot process this kind of PCI device
        //
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Program the PCI device, to override the PCI features as per the policy,
  resolved from previous traverse.

  @param RootBridge             A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           The other PCI features configuration during enumeration
                                of all the nodes of the PCI root bridge instance were
                                programmed in PCI-compliance pattern along with the
                                device-specific policy, as applicable.
  @retval EFI_UNSUPPORTED       One of the override operation maong the nodes of
                                the PCI hierarchy resulted in a incompatible address
                                range.
  @retval EFI_INVALID_PARAMETER The override operation is performed with invalid input
                                parameters.
**/
EFI_STATUS
ProgramDevicePciFeatures (
  IN PCI_IO_DEVICE          *PciDevice
  )
{
  EFI_STATUS           Status;

  if ( SetupMaxPayloadSize ()) {
    Status = OverrideMaxPayloadSize (PciDevice);
  }
  if ( SetupMaxReadReqSize ()) {
    Status = OverrideMaxReadReqSize (PciDevice);
  }
  return Status;
}

/**
  Program all the nodes of the specified root bridge or PCI-PCI Bridge, to
  override the PCI features.

  @param RootBridge             A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           The other PCI features configuration during enumeration
                                of all the nodes of the PCI root bridge instance were
                                programmed in PCI-compliance pattern along with the
                                device-specific policy, as applicable.
  @retval EFI_UNSUPPORTED       One of the override operation maong the nodes of
                                the PCI hierarchy resulted in a incompatible address
                                range.
  @retval EFI_INVALID_PARAMETER The override operation is performed with invalid input
                                parameters.
**/
EFI_STATUS
ProgramPciFeatures (
  IN PCI_IO_DEVICE          *RootBridge
  )
{
  EFI_STATUS           Status;
  LIST_ENTRY           *Link;
  PCI_IO_DEVICE        *Device;

  for ( Link = RootBridge->ChildList.ForwardLink
      ; Link != &RootBridge->ChildList
      ; Link = Link->ForwardLink
  ) {
    Device = PCI_IO_DEVICE_FROM_LINK (Link);
    if (IS_PCI_BRIDGE (&Device->Pci)) {
      DEBUG ((
          DEBUG_INFO, "::Bridge [%02x|%02x|%02x] -",
          Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
      ));
      if (Device->IsPciExp) {
        DEBUG (( DEBUG_INFO, "ready to override!\n"));

        Status = ProgramDevicePciFeatures ( Device);
      } else {
        DEBUG (( DEBUG_INFO, "skipped!\n"));
        //
        // PCI Bridge which does not have PCI Express Capability structure
        // cannot process this kind of PCI Bridge device
        //

      }

      Status = ProgramPciFeatures ( Device);
    } else {
      DEBUG ((
          DEBUG_INFO, "::Device [%02x|%02x|%02x] -",
          Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
      ));
      if (Device->IsPciExp) {
        DEBUG (( DEBUG_INFO, "ready to override!\n"));

        Status = ProgramDevicePciFeatures ( Device);
      } else {
        DEBUG (( DEBUG_INFO, "skipped!\n"));
        //
        // PCI Device which does not have PCI Express Capability structure
        // cannot process this kind of PCI device
        //
      }
    }
  }

  return Status;
}

/**
  Create a node of type PRIMARY_ROOT_PORT_NODE for the given PCI device, and
  assigns EFI handles of its Root Bridge and its own, along with its PCI Bus
  range for the secondary and subordinate bus range.

  @param  RootBridge          A pointer to the PCI_IO_DEVICE for its PCI Root Bridge
  @param  Device              A pointer to the PCI_IO_DEVICE for the PCI controller
  @param  RootPortSecBus      PCI controller's Secondary Bus number
  @param  RootPortSubBus      PCI controller's Subordinate Bus number
  @param  PrimaryRootPortNode A pointer to the PRIMARY_ROOT_PORT_NODE to return
                              the newly created node for the PCI controller. In
                              case of error nothing is return in this.

  @retval EFI_SUCCESS           new node of PRIMARY_ROOT_PORT_NODE is returned for
                                the PCI controller
          EFI_OUT_OF_RESOURCES  unable to create the node for the PCI controller
          EFI_INVALID_PARAMETER unable to store the node as the input buffer is
                                not empty (*PrimaryRootPortNode)
**/
EFI_STATUS
CreatePrimaryPciRootPortNode (
  IN  PCI_IO_DEVICE           *RootBridge,
  IN  PCI_IO_DEVICE           *Device,
  IN  UINT8                   RootPortSecBus,
  IN  UINT8                   RootPortSubBus,
  OUT PRIMARY_ROOT_PORT_NODE  **PrimaryRootPortNode
  )
{
  PRIMARY_ROOT_PORT_NODE      *RootPortNode = NULL;

  if ( !*PrimaryRootPortNode) {
    RootPortNode                    = AllocateZeroPool ( sizeof (PRIMARY_ROOT_PORT_NODE));
    if ( RootPortNode == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    RootPortNode->Signature         = PCI_ROOT_PORT_SIGNATURE;
    RootPortNode->RootBridgeHandle  = RootBridge->Handle;
    RootPortNode->RootPortHandle    = Device->Handle;
    RootPortNode->SecondaryBusStart = RootPortSecBus;
    RootPortNode->SecondaryBusEnd   = RootPortSubBus;
    InitializeListHead ( &RootPortNode->NeighborRootPort);
    *PrimaryRootPortNode = RootPortNode;
    return EFI_SUCCESS;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

/**
  Checks to report whether the input PCI controller's secondary / subordinate
  bus numbers are within the recorded list of other PCI controllers (root ports).

  @param  RootPortNode      A pointer to the first node of PRIMARY_ROOT_PORT_NODE
  @param  RootPortSecBus    PCI secondary bus number of the PCI controller found
  @param  RootPortSubBus    PCI subordinate bus number of the PCI Root Port found

  @retval TRUE              A child PCI Root port found
          FALSE             A new PCI controller found
**/
BOOLEAN
CheckChildRootPort (
  IN      PRIMARY_ROOT_PORT_NODE  *RootPortNode,
  IN      UINT8                   RootPortSecBus,
  IN      UINT8                   RootPortSubBus
)
{
  LIST_ENTRY              *Link;
  PRIMARY_ROOT_PORT_NODE  *Temp;

  if ( !RootPortNode) {
    return FALSE;
  }
  Link = &RootPortNode->NeighborRootPort;
  do {
    Temp = PRIMARY_ROOT_PORT_NODE_FROM_LINK ( Link);
    if ( RootPortSecBus >= Temp->SecondaryBusStart
        && RootPortSubBus <= Temp->SecondaryBusEnd) {
      //
      // given root port's secondary & subordinate within its primary ports
      // hence return as child port
      //
      return TRUE;
    }
    Link = Link->ForwardLink;
  } while (Link != &RootPortNode->NeighborRootPort);
  //
  // the given root port's secondary / subordinate bus numbers do not belong to
  // any existing primary root port's bus range hence consider another primary
  // root port of the root bridge controller
  //
  return FALSE;
}

/**
  Create the vector of PCI Feature configuration table as per the number of
  the PCI Root Ports given, assigns default value to the PCI features supported
  and assign its address to the global variable "mPciFeaturesConfigurationTableInstances".

  @param  NumberOfRootPorts   An input arguement of UINTN to indicate number of
                              primary PCI physical Root Bridge devices found

  @retval EFI_OUT_OF_RESOURCES  unable to allocate buffer to store PCI feature
                                configuration table for all the physical PCI root
                                ports given
          EFI_SUCCESS           PCI Feature COnfiguration table created for all
                                the PCI Rooot ports reported
 */
EFI_STATUS
CreatePciFeaturesConfigurationTableInstances (
  IN  UINTN NumberOfRootPorts
  )
{
  OTHER_PCI_FEATURES_CONFIGURATION_TABLE   *PciRootBridgePortFeatures = NULL;
  UINTN                                     Instances;

  PciRootBridgePortFeatures = AllocateZeroPool (
                                sizeof ( OTHER_PCI_FEATURES_CONFIGURATION_TABLE) * NumberOfRootPorts
                                );
  if ( !PciRootBridgePortFeatures) {
    return EFI_OUT_OF_RESOURCES;
  }

  for ( Instances = 0; Instances < NumberOfRootPorts; Instances++) {
    PciRootBridgePortFeatures [Instances].ID                    = Instances + 1;
    PciRootBridgePortFeatures [Instances].Max_Payload_Size      = PCIE_MAX_PAYLOAD_SIZE_4096B;
    PciRootBridgePortFeatures [Instances].Max_Read_Request_Size = PCIE_MAX_READ_REQ_SIZE_4096B;
    PciRootBridgePortFeatures [Instances].Lock_Max_Read_Request_Size = FALSE;
  }
  mPciFeaturesConfigurationTableInstances = PciRootBridgePortFeatures;
  return EFI_SUCCESS;
}

/**
  This routine pairs the each PCI Root Port node with one of the PCI Feature
  Configuration Table node. Each physical PCI Root Port has its own PCI feature
  configuration table which will used for aligning all its downstream components.

  @param  NumberOfRootPorts     inputs the number of physical PCI root ports
                                found on the Root bridge instance

  @retval EFI_INVALID_PARAMETER if the primary PCI root ports list is vacant when
                                there is one or more PCI Root port indicated as per
                                input parameter
          EFI_UNSUPPORTED       The PCI Root Port nodes not paired equally with
                                the PCI Configuration Table nodes
          EFI_SUCCESS           each PCI feature configuration node is paired equally
                                with each PCI Root port in the list
**/
EFI_STATUS
AssignPciFeaturesConfigurationTable (
  IN  UINTN NumberOfRootPorts
  )
{
  UINTN                       Instances;
  LIST_ENTRY                  *Link;
  PRIMARY_ROOT_PORT_NODE      *Temp;

  if ( !mPrimaryRootPortList
      && NumberOfRootPorts) {
    DEBUG ((
        DEBUG_ERROR,
        "Critical error! no internal table setup for %d PCI Root ports \n",
        NumberOfRootPorts
    ));
    return EFI_INVALID_PARAMETER;
  }

  if ( NumberOfRootPorts) {
    Link = &mPrimaryRootPortList->NeighborRootPort;
    for ( Instances = 0
        ; (Instances < NumberOfRootPorts)
        ; Instances++
    ) {
      Temp = PRIMARY_ROOT_PORT_NODE_FROM_LINK ( Link);
      Temp->OtherPciFeaturesConfigurationTable = &mPciFeaturesConfigurationTableInstances [Instances];
      DEBUG ((
          DEBUG_INFO,
          "Assigned to %dth primary root port\n",
          Instances
      ));

      Link = Link->ForwardLink;
    }
    if ( Link != &mPrimaryRootPortList->NeighborRootPort) {
      DEBUG ((
          DEBUG_ERROR,
          "Error!! PCI Root Port list is not properly matched with Config., Table list \n"
      ));
      return EFI_UNSUPPORTED;
    }
  }
  return  EFI_SUCCESS;
}

/**
  Prepare each PCI Controller (Root Port) with its own PCI Feature configuration
  table node that can be used for tracking to align all PCI nodes in its hierarchy.

  @param  PrimaryRootPorts      A pointer to PRIMARY_ROOT_PORT_NODE
  @param  NumberOfRootPorts     Total number of pysical primary PCI Root ports

  @retval EFI_OUT_OF_RESOURCES  unable to allocate buffer to store PCI feature
                                configuration table for all the physical PCI root
                                ports given
          EFI_INVALID_PARAMETER if the primary PCI root ports list is vacant when
                                there is one or more PCI Root port indicated as per
                                input parameter
          EFI_UNSUPPORTED       The PCI Root Port nodes not paired equally with
                                the PCI Configuration Table nodes
          EFI_SUCCESS           each PCI feature configuration node is paired equally
                                with each PCI Root port in the list
**/
EFI_STATUS
PreparePciControllerConfigurationTable (
  IN  PRIMARY_ROOT_PORT_NODE    *PrimaryRootPorts,
  IN  UINTN                     NumberOfRootPorts
  )
{
  EFI_STATUS                    Status;

  mPrimaryRootPortList = PrimaryRootPorts;
  DEBUG ((
      DEBUG_INFO, "Number of primary Root Ports found on this bridge = %d\n",
      NumberOfRootPorts
  ));

  Status = CreatePciFeaturesConfigurationTableInstances ( NumberOfRootPorts);
  if ( EFI_ERROR(Status)) {
    DEBUG ((
        DEBUG_ERROR, "Unexpected memory node creation error for PCI features!\n"
    ));
  } else {
    //
    // align the primary root port nodes list with the PCI Feature configuration
    // table. Note that the PCI Feature configuration table is not maintain for
    // the RCiEP devices
    //
    Status = AssignPciFeaturesConfigurationTable ( NumberOfRootPorts);
  }
  return Status;
}

/**
  Scan all the nodes of the RootBridge to identify and create a separate list
  of all primary physical PCI root ports and link each with its own instance of
  the PCI Feature Configuration Table.

  @param  RootBridge    A pointer to the PCI_IO_DEVICE of the PCI Root Bridge

  @retval EFI_OUT_OF_RESOURCES  unable to allocate buffer to store PCI feature
                                configuration table for all the physical PCI root
                                ports given
          EFI_NOT_FOUND         No PCI Bridge device found
          EFI_SUCCESS           PCI Feature COnfiguration table created for all
                                the PCI Rooot ports found
          EFI_INVALID_PARAMETER invalid parameter passed to the routine which
                                creates the PCI controller node for the primary
                                Root post list
**/
EFI_STATUS
RecordPciRootPortBridges (
  IN  PCI_IO_DEVICE           *RootBridge
  )
{
  EFI_STATUS              Status = EFI_NOT_FOUND;
  LIST_ENTRY              *Link;
  PCI_IO_DEVICE           *Device;
  UINTN                   NumberOfRootPorts;
  PRIMARY_ROOT_PORT_NODE  *PrimaryRootPorts,
                          *TempNode;
  UINT8                   RootPortSecBus,
                          RootPortSubBus;

  DEBUG ((
      DEBUG_INFO, "<<********** RecordPciRootPortBridges -start *************>>\n"
  ));
  NumberOfRootPorts = 0;
  PrimaryRootPorts = NULL;
  for ( Link = RootBridge->ChildList.ForwardLink
      ; Link != &RootBridge->ChildList
      ; Link = Link->ForwardLink
  ) {
    Device = PCI_IO_DEVICE_FROM_LINK (Link);
    if (IS_PCI_BRIDGE (&Device->Pci)) {
      Status = GetPciRootPortBusAssigned (
                  Device,
                  NULL,
                  &RootPortSecBus,
                  &RootPortSubBus
                  );
      if ( !EFI_ERROR(Status)) {
        DEBUG ((
            DEBUG_INFO, "::Device [%02x|%02x|%02x] - SecBus=0x%x, SubBus=0x%x\n",
            Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber,
            RootPortSecBus, RootPortSubBus
        ));
      } else {
        DEBUG ((
            DEBUG_ERROR, "Unexpected read error [0x%lx]::Device [%02x|%02x|%02x]\n",
            Status, Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
        ));
        RootPortSecBus = RootPortSubBus = 0;
        continue;
      }

      if ( !PrimaryRootPorts) {
        NumberOfRootPorts++;
        Status = CreatePrimaryPciRootPortNode (
                    RootBridge,
                    Device,
                    RootPortSecBus,
                    RootPortSubBus,
                    &PrimaryRootPorts
                    );
        if ( EFI_ERROR(Status)) {
          //
          // abort mission to scan for all primary roots ports of a bridge
          // controller if error encountered for very first PCI primary root port
          //
          DEBUG ((
              DEBUG_ERROR, "Unexpected node creation error [0x%lx]::Device [%02x|%02x|%02x]\n",
              Status, Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
          ));
          return Status;
        }
        DEBUG ((
            DEBUG_INFO, "first primary root port found::Device [%02x|%02x|%02x]\n",
            Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
        ));
      } else {
        if ( !CheckChildRootPort ( PrimaryRootPorts, RootPortSecBus, RootPortSubBus)) {
          NumberOfRootPorts++;
          TempNode = NULL;
          Status = CreatePrimaryPciRootPortNode (
                      RootBridge,
                      Device,
                      RootPortSecBus,
                      RootPortSubBus,
                      &TempNode
                      );
          if ( !EFI_ERROR(Status)) {
            //
            // another primary root port found on the same bridge controller
            // insert in the node list
            //
            InsertTailList ( &PrimaryRootPorts->NeighborRootPort, &TempNode->NeighborRootPort);
            DEBUG ((
                DEBUG_INFO, "next primary root port found::Device [%02x|%02x|%02x]\n",
                Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
            ));
          } else {
            DEBUG ((
                DEBUG_ERROR, "Unexpected node creation error [0x%lx]::Device [%02x|%02x|%02x]\n",
                Status, Device->BusNumber, Device->DeviceNumber, Device->FunctionNumber
            ));
          }
        }
      }
    }
  }
  //
  // prepare the PCI root port and its feature configuration table list
  //
  if ( NumberOfRootPorts) {
    Status = PreparePciControllerConfigurationTable (
                PrimaryRootPorts,
                NumberOfRootPorts
              );

  } else {
    DEBUG ((
        DEBUG_INFO, "No PCI Root port found on this bridge!\n"
    ));
  }

  DEBUG ((
      DEBUG_INFO, "<<********** RecordPciRootPortBridges - end **********>>\n"
  ));
  return Status;
}

/**
  Enumerate all the nodes of the specified root bridge or PCI-PCI Bridge, to
  configure the other PCI features.

  @param RootBridge          A pointer to the PCI_IO_DEVICE.

  @retval EFI_SUCCESS           The other PCI features configuration during enumeration
                                of all the nodes of the PCI root bridge instance were
                                programmed in PCI-compliance pattern along with the
                                device-specific policy, as applicable.
  @retval EFI_UNSUPPORTED       One of the override operation maong the nodes of
                                the PCI hierarchy resulted in a incompatible address
                                range.
  @retval EFI_INVALID_PARAMETER The override operation is performed with invalid input
                                parameters.
**/
EFI_STATUS
EnumerateOtherPciFeatures (
  IN PCI_IO_DEVICE          *RootBridge
  )
{
  EFI_STATUS            Status;
  CHAR16                *Str;
  UINTN                 OtherPciFeatureConfigPhase;

  //
  // check on PCI features configuration is complete and re-enumeration is required
  //
  if ( !CheckPciFeaturesConfigurationRequired ( RootBridge)) {
    return EFI_ALREADY_STARTED;
  }

  Str = ConvertDevicePathToText (
          DevicePathFromHandle (RootBridge->Handle),
          FALSE,
          FALSE
        );
  DEBUG ((DEBUG_INFO, "Enumerating PCI features for Root Bridge %s\n", Str != NULL ? Str : L""));

  for ( OtherPciFeatureConfigPhase = PciFeatureRootBridgeScan
      ; OtherPciFeatureConfigPhase <= PciFeatureConfigurationComplete
      ; OtherPciFeatureConfigPhase++
      ) {
    switch ( OtherPciFeatureConfigPhase){
      case  PciFeatureRootBridgeScan:
        SetupPciFeaturesConfigurationDefaults ();
        //
        //first scan the entire root bridge heirarchy for the primary PCI root ports
        //
        RecordPciRootPortBridges ( RootBridge);
        break;

      case  PciFeatureGetDevicePolicy:
      case  PciFeatureSetupPhase:
        DEBUG ((
            DEBUG_INFO, "<<********** SetupPciFeatures - start **********>>\n"
        ));
        //
        // enumerate the other PCI features
        //
        Status = SetupPciFeatures ( RootBridge, OtherPciFeatureConfigPhase);

        DEBUG ((
            DEBUG_INFO, "<<********** SetupPciFeatures - end **********>>\n"
        ));
        break;

      case  PciFeatureConfigurationPhase:
        //
        // override the PCI features as per enumeration phase
        //
        DEBUG ((DEBUG_INFO, "PCI features override for Root Bridge %s\n", Str != NULL ? Str : L""));
        DEBUG ((
            DEBUG_INFO, "<<********** ProgramPciFeatures - start **********>>\n"
        ));
        Status = ProgramPciFeatures ( RootBridge);
        DEBUG ((
            DEBUG_INFO, "<<********** ProgramPciFeatures - end **********>>\n"
        ));
        break;

      case  PciFeatureConfigurationComplete:
        //
        // clean up the temporary resource nodes created for this root bridge
        //
        DestroyPrimaryRootPortNodes ();

        ErasePciFeaturesConfigurationTable ();
    }
  }

  if (Str != NULL) {
    FreePool (Str);
  }
  //
  // mark this root bridge as PCI features configuration complete, and no new
  // enumeration is required
  //
  AddRootBridgeInPciFeaturesConfigCompletionList ( RootBridge, FALSE);
  return Status;
}

/**
  This routine is invoked from the Stop () interface for the EFI handle of the
  RootBridge. Free up its node of type PCI_FEATURE_CONFIGURATION_COMPLETION_LIST.

  @param  RootBridge      A pointer to the PCI_IO_DEVICE
**/
VOID
DestroyRootBridgePciFeaturesConfigCompletionList (
  IN PCI_IO_DEVICE          *RootBridge
  )
{
  LIST_ENTRY                                  *Link;
  PCI_FEATURE_CONFIGURATION_COMPLETION_LIST   *Temp;

  if ( mPciFeaturesConfigurationCompletionList) {
    Link = &mPciFeaturesConfigurationCompletionList->RootBridgeLink;

    do {
      Temp = PCI_FEATURE_CONFIGURATION_COMPLETION_LIST_FROM_LINK (Link);
      if ( Temp->RootBridgeHandle == RootBridge->Handle) {
        RemoveEntryList ( Link);
        FreePool ( Temp);
        return;
      }
      Link = Link->ForwardLink;
    } while (Link != &mPciFeaturesConfigurationCompletionList->RootBridgeLink);
  }
  //
  // not found on the PCI feature configuration completion list, return
  //
  return;
}
