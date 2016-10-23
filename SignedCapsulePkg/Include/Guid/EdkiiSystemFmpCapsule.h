/** @file
  Guid & data structure used for Delivering Capsules Containing Updates to
  EDKII System Firmware Management Protocol

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#ifndef __EDKII_SYSTEM_FMP_CAPSULE_GUID_H__
#define __EDKII_SYSTEM_FMP_CAPSULE_GUID_H__

/**

  Capsule Layout is below:
  +------------------------------------------+
  |    Capsule Header (OPTIONAL, WFU)        | <== ESRT.FwClass (Optional)
  +------------------------------------------+
  |          FMP Capsule Header              | <== EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID
  +------------------------------------------+
  | FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER | <== PcdSystemFmpCapsuleImageTypeIdGuid
  +------------------------------------------+
  |     EFI_FIRMWARE_IMAGE_AUTHENTICATION    |
  +------------------------------------------+
  |             FMP Payload                  |
  +------------------------------------------+

  System FMP Payload is below:
  +------------------------------------------+
  |            EFI_FIRMWARE_VOLUME           |
  |  +------------------------------------+  |
  |  |       FFS (Configure File)         |  | <== gEdkiiSystemFmpCapsuleConfigFileGuid
  |  +------------------------------------+  |
  |  |         FFS (Driver FV)            |  | <== gEdkiiSystemFmpCapsuleDriverFvFileGuid
  |  +------------------------------------+  |
  |  |    FFS (System Firmware Image)     |  | <== PcdEdkiiSystemFirmwareFileGuid
  |  |  +------------------------------+  |  |
  |  |  |          FV Recovery         |  |  |
  |  |  |------------------------------|  |  |
  |  |  |           Fv Main            |  |  |
  |  |  +------------------------------+  |  |
  |  +------------------------------------+  |
  +------------------------------------------+

**/

#define EDKII_SYSTEM_FIRMWARE_IMAGE_DESCRIPTOR_SIGNATURE  SIGNATURE_32('S', 'F', 'I', 'D')

#pragma pack(1)
typedef struct {
  UINT32                                Signature;
  UINT32                                HeaderLength; // Length of EDKII_SYSTEM_FIRMWARE_IMAGE_DESCRIPTOR, excluding NameString
  UINT32                                Length;       // Length of the data structure, including NameString
  // Below structure is similar as UEFI EFI_FIRMWARE_MANAGEMENT_PROTOCOL.GetPackageInfo()
  UINT32                                PackageVersion;
  UINT32                                PackageVersionNameStringOffset; // Offset from head, CHAR16 string including NULL terminate char
  // Below structure is similar as UEFI EFI_FIRMWARE_IMAGE_DESCRIPTOR
  UINT8                                 ImageIndex;
  UINT8                                 Reserved[3];
  EFI_GUID                              ImageTypeId;
  UINT64                                ImageId;
  UINT32                                ImageIdNameStringOffset; // Offset from head, CHAR16 string including NULL terminate char
  UINT32                                Version;
  UINT32                                VersionNameStringOffset; // Offset from head, CHAR16 string including NULL terminate char
  UINT8                                 Reserved2[4];
  UINT64                                Size;
  UINT64                                AttributesSupported;
  UINT64                                AttributesSetting;
  UINT64                                Compatibilities;
  UINT32                                LowestSupportedImageVersion;
  UINT32                                LastAttemptVersion;
  UINT32                                LastAttemptStatus;
  UINT8                                 Reserved3[4];
  UINT64                                HardwareInstance;
  // real string data
//CHAR16                                ImageIdNameStr[];        // CHAR16 string including NULL terminate char
//CHAR16                                VersionNameStr[];        // CHAR16 string including NULL terminate char
//CHAR16                                PackageVersionNameStr[]; // CHAR16 string including NULL terminate char
} EDKII_SYSTEM_FIRMWARE_IMAGE_DESCRIPTOR;
#pragma pack()

/**
  System Firmware Image Descriptor is below:
            +----------------------+
            | System Firmware (FV) |
            |+--------------------+|
            ||   FFS (Freeform)   || <== gEdkiiSystemFirmwareImageDescriptorFileGuid
            ||+------------------+||
            |||   SECTION (RAW)  |||
            |||  System Firmware |||
            ||| Image Descriptor |||
            ||+------------------+||
            |+--------------------+|
            |                      |
            |                      |
            +----------------------+
**/

extern EFI_GUID gEdkiiSystemFirmwareImageDescriptorFileGuid;
extern EFI_GUID gEdkiiSystemFmpCapsuleConfigFileGuid;
extern EFI_GUID gEdkiiSystemFmpCapsuleDriverFvFileGuid;

#endif
