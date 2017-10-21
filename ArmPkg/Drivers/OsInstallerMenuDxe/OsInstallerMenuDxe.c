/** @file
 *
 *  Copyright (c) 2016-2017, Linaro Limited. All rights reserved.
 *
 *  This program and the accompanying materials are licensed and made available
 *  under the terms and conditions of the BSD License which accompanies this
 *  distribution.  The full text of the license may be found at
 *  http://opensource.org/licenses/bsd-license.php
 *
 *  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 */

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/SimpleNetwork.h>

typedef struct {
  CONST CHAR16  *Name;
  CONST CHAR8   *Uri;
} OS_INSTALLER_IMAGE;

STATIC CONST OS_INSTALLER_IMAGE mOsInstallers[] = {
  { L"Install Debian Stretch over HTTP",
    "http://ftp.us.debian.org/debian/dists/stretch/main/installer-arm64/current/images/netboot/mini.iso" },

  { L"Install Ubuntu 17.10 (Artful) over HTTP",
    "http://ports.ubuntu.com/ubuntu-ports/dists/artful/main/installer-arm64/current/images/netboot/mini.iso" },

//
// The links below refer to 300-500 MB netboot images that need to be exposed to
// the OS via a ramdisk after the OS loader boots the installer from it.
// Currently, this requires ACPI/NFIT support, which was only enabled for arm64
// in Linux in version v4.14. For DT boot, there is currently no solution for
// this.
//
//  { L"Install openSUSE Tumbleweed over HTTP",
//    "http://download.opensuse.org/ports/aarch64/factory/iso/openSUSE-Tumbleweed-NET-aarch64-Current.iso" },
//
//  { L"Install Fedora Server 26 over HTTP",
//    "http://download.fedoraproject.org/pub/fedora-secondary/releases/26/Server/aarch64/iso/Fedora-Server-netinst-aarch64-26-1.5.iso" },
//
//  { L"Install Centos 7 over HTTP",
//    "http://mirror.centos.org/altarch/7/isos/aarch64/CentOS-7-aarch64-NetInstall.iso" }
//
};

STATIC EFI_EVENT                  mRegisterProtocolEvent;
STATIC VOID                       *mRegistration;

STATIC
EFI_STATUS
CreateOsInstallerBootOptions (
  IN  EFI_HANDLE    Handle
  )
{
  EFI_DEVICE_PATH_PROTOCOL        *ParentDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *TmpDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *NewDevicePath;
  EFI_STATUS                      Status;
  UINTN                           Idx;
  EFI_DEV_PATH                    *Node;
  UINTN                           Length;
  EFI_BOOT_MANAGER_LOAD_OPTION    NewOption;
  EFI_BOOT_MANAGER_LOAD_OPTION    *BootOptions;
  UINTN                           BootOptionCount;
  INTN                            OptionIndex;

  BootOptions = EfiBootManagerGetLoadOptions (&BootOptionCount,
                  LoadOptionTypeBoot);
  ASSERT (BootOptions != NULL);

  Status = gBS->HandleProtocol (Handle, &gEfiDevicePathProtocolGuid,
                  (VOID **)&ParentDevicePath);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "%a: gBS->HandleProtocol returned %r\n",
      __FUNCTION__, Status));
    return Status;
  }

  for (Idx = 0; Idx < ARRAY_SIZE (mOsInstallers); Idx++) {
    Node = AllocateZeroPool (sizeof (IPv4_DEVICE_PATH));
    if (Node == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto FreeBootOptions;
    }
    Node->Ipv4.Header.Type    = MESSAGING_DEVICE_PATH;
    Node->Ipv4.Header.SubType = MSG_IPv4_DP;
    SetDevicePathNodeLength (Node, sizeof (IPv4_DEVICE_PATH));

    TmpDevicePath = AppendDevicePathNode (ParentDevicePath,
                      (EFI_DEVICE_PATH_PROTOCOL*) Node);
    FreePool (Node);
    if (TmpDevicePath == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto FreeBootOptions;
    }

    //
    // Update the URI node with the input boot file URI.
    //
    Length = sizeof (EFI_DEVICE_PATH_PROTOCOL) +
             AsciiStrSize (mOsInstallers[Idx].Uri);
    Node = AllocatePool (Length);
    if (Node == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      FreePool (TmpDevicePath);
      goto FreeBootOptions;
    }
    Node->DevPath.Type    = MESSAGING_DEVICE_PATH;
    Node->DevPath.SubType = MSG_URI_DP;
    SetDevicePathNodeLength (Node, Length);
    CopyMem ((UINT8*) Node + sizeof (EFI_DEVICE_PATH_PROTOCOL),
      mOsInstallers[Idx].Uri, AsciiStrSize (mOsInstallers[Idx].Uri));
    NewDevicePath = AppendDevicePathNode (TmpDevicePath,
                      (EFI_DEVICE_PATH_PROTOCOL*) Node);
    FreePool (Node);
    FreePool (TmpDevicePath);
    if (NewDevicePath == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto FreeBootOptions;
    }

    //
    // Create a new load option.
    //
    Status = EfiBootManagerInitializeLoadOption (&NewOption,
               LoadOptionNumberUnassigned, LoadOptionTypeBoot,
               LOAD_OPTION_ACTIVE, (CHAR16 *)mOsInstallers[Idx].Name,
               NewDevicePath, NULL, 0);
    ASSERT_EFI_ERROR (Status);

    OptionIndex = EfiBootManagerFindLoadOption (&NewOption, BootOptions,
                    BootOptionCount);
    if (OptionIndex == -1) {
      //
      // Add the new load option if it did not exist already
      //
      EfiBootManagerAddLoadOptionVariable (&NewOption, (UINTN) -1);
    }
    EfiBootManagerFreeLoadOption (&NewOption);
    FreePool (NewDevicePath);
  }

FreeBootOptions:

  EfiBootManagerFreeLoadOptions (BootOptions, BootOptionCount);
  return Status;
}

STATIC
BOOLEAN
MediaDisconnected (
  IN  EFI_HANDLE    Handle
  )
{
  EFI_SIMPLE_NETWORK_PROTOCOL   *Snp;
  EFI_STATUS                    Status;

  Status = gBS->HandleProtocol (Handle, &gEfiSimpleNetworkProtocolGuid,
                  (VOID **)&Snp);
  if (EFI_ERROR (Status) || !Snp->Mode->MediaPresentSupported) {
    return FALSE;
  }

  Snp->GetStatus (Snp, NULL, NULL);

  return !Snp->Mode->MediaPresent;
}

STATIC
VOID
OnRegisterProtocol (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                      Status;
  UINTN                           HandleCount;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                           Idx;

  Status = gBS->LocateHandleBuffer (ByRegisterNotify,
                  &gEfiHttpServiceBindingProtocolGuid, mRegistration,
                  &HandleCount, &HandleBuffer);
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Idx = 0; Idx < HandleCount; Idx++) {
    if (MediaDisconnected (HandleBuffer[Idx])) {
      continue;
    }

    CreateOsInstallerBootOptions (HandleBuffer[Idx]);

    //
    // Create the options only a single time - we take care to only install
    // them for a network interface that has a link, and we should try not to
    // confuse the user by having 10 identical options when the system has 10
    // network interfaces.
    //
    gBS->CloseEvent (Event);
    break;
  }
  FreePool (HandleBuffer);
}

EFI_STATUS
EFIAPI
OsInstallerMenuDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  mRegisterProtocolEvent = EfiCreateProtocolNotifyEvent (
                             &gEfiHttpServiceBindingProtocolGuid, TPL_CALLBACK,
                             OnRegisterProtocol, NULL, &mRegistration);

  return EFI_SUCCESS;
}
