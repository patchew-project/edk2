/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Common.h"
#include "VirtualGpu.h"

EFI_STATUS
EFIAPI
GvtGopComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );

EFI_STATUS
EFIAPI
GvtGopComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL *This,
  IN  EFI_HANDLE                  ControllerHandle,
  IN  EFI_HANDLE                  ChildHandle OPTIONAL,
  IN  CHAR8                       *Language,
  OUT CHAR16                      **ControllerName
  );

GLOBAL_REMOVE_IF_UNREFERENCED
EFI_COMPONENT_NAME_PROTOCOL gGvtGopDriverComponentName = {
  GvtGopComponentNameGetDriverName,
  GvtGopComponentNameGetControllerName,
  "eng"
};

GLOBAL_REMOVE_IF_UNREFERENCED
EFI_COMPONENT_NAME2_PROTOCOL gGvtGopDriverComponentName2 = {
  (EFI_COMPONENT_NAME2_GET_DRIVER_NAME) GvtGopComponentNameGetDriverName,
  (EFI_COMPONENT_NAME2_GET_CONTROLLER_NAME) GvtGopComponentNameGetControllerName,
  "en"
};


GLOBAL_REMOVE_IF_UNREFERENCED
EFI_UNICODE_STRING_TABLE gGvtGopDriverNameTable[] = {
  { "eng;en", L"Intel GVT-g GOP Driver" },
  { NULL , NULL }
};

GLOBAL_REMOVE_IF_UNREFERENCED
EFI_UNICODE_STRING_TABLE gGvtGopControllerNameTable[] = {
  { "eng;en", L"Intel GVT-g Virtual GPU PCI Adapter" },
  { NULL , NULL }
};

STATIC
EFI_STATUS
EFIAPI
GvtGopBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                  ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_VERBOSE,
      "OpenProtocol gEfiPciIoProtocolGuid failed with %d\n", Status
      );
    goto Done;
  }

  Status = IntelVirtualGpuActive (PciIo);

  gBS->CloseProtocol (
        ControllerHandle,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        ControllerHandle
        );

Done:
  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);
  return Status;
}


STATIC
EFI_STATUS
EFIAPI
GvtGopBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                  ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS               Status;
  EFI_TPL                  OriginalTPL;
  GVT_GOP_PRIVATE_DATA     *GvtGopPrivate = NULL;
  UINT64                   PciAttr;
  EFI_DEVICE_PATH_PROTOCOL *ParentDevicePath;
  ACPI_ADR_DEVICE_PATH     AcpiDeviceNode;
  EFI_PCI_IO_PROTOCOL      *ChildPciIo;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  OriginalTPL = gBS->RaiseTPL (TPL_CALLBACK);

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(GVT_GOP_PRIVATE_DATA),
                  (VOID **)&GvtGopPrivate
                  );
  if (EFI_ERROR (Status) || GvtGopPrivate == NULL) {
    GVT_DEBUG (EFI_D_ERROR,
      "AllocatePool failed for GVT_GOP_PRIVATE_DATA, size %d, status %d\n",
      sizeof(GVT_GOP_PRIVATE_DATA), Status
      );
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  ZeroMem (GvtGopPrivate, sizeof(GVT_GOP_PRIVATE_DATA));
  GvtGopPrivate->Signature = GVT_GOP_MAGIC;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(INTEL_VIRTUAL_GPU),
                  &GvtGopPrivate->VirtualGpu
                  );
  if (EFI_ERROR (Status) || GvtGopPrivate->VirtualGpu == NULL) {
    GVT_DEBUG (EFI_D_ERROR,
      "AllocatePool failed for INTEL_VIRTUAL_GPU, size %d, status %d\n",
      sizeof(INTEL_VIRTUAL_GPU), Status
      );
    Status = EFI_OUT_OF_RESOURCES;
    goto Free;
  }
  ZeroMem (GvtGopPrivate->VirtualGpu, sizeof(INTEL_VIRTUAL_GPU));

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &GvtGopPrivate->PciIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't open protocol gEfiPciIoProtocolGuid, status %d\n", Status
      );
    goto Free;
  }

  Status = GvtGopPrivate->PciIo->Attributes (
                                   GvtGopPrivate->PciIo,
                                   EfiPciIoAttributeOperationGet,
                                   0,
                                   &GvtGopPrivate->OriginalPciAttr
                                   );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Failed EfiPciIoAttributeOperationGet, status %d\n", Status
      );
    goto Free;
  }

  PciAttr = EFI_PCI_DEVICE_ENABLE;
  Status = GvtGopPrivate->PciIo->Attributes (
                                   GvtGopPrivate->PciIo,
                                   EfiPciIoAttributeOperationEnable,
                                   PciAttr,
                                   NULL);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Failed EfiPciIoAttributeOperationEnable %llx, status %d\n", PciAttr,
      Status
      );
    goto Free;
  }

  Status = IntelVirtualGpuInit (GvtGopPrivate);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR, "Failed IntelVirtualGpuInit, status %d\n", Status);
    goto Free;
  }

  Status = gBS->HandleProtocol (
                  ControllerHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath
                  );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR, "Fail gEfiDevicePathProtocolGuid, status %d\n",
      Status
      );
    goto Free;
  }

  ZeroMem (&AcpiDeviceNode, sizeof (ACPI_ADR_DEVICE_PATH));
  AcpiDeviceNode.Header.Type = ACPI_DEVICE_PATH;
  AcpiDeviceNode.Header.SubType = ACPI_ADR_DP;
  AcpiDeviceNode.ADR = ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_EXTERNAL_DIGITAL, 0, 0);
  SetDevicePathNodeLength (&AcpiDeviceNode.Header, sizeof (ACPI_ADR_DEVICE_PATH));
  GvtGopPrivate->GopDevPath = AppendDevicePathNode (
                                ParentDevicePath,
                                (EFI_DEVICE_PATH_PROTOCOL *) &AcpiDeviceNode
                                );
  if (GvtGopPrivate->GopDevPath == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    GVT_DEBUG (EFI_D_ERROR, "Fail AppendDevicePathNode, status %d\n", Status);
    goto Free;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &GvtGopPrivate->Handle,
                  &gEfiDevicePathProtocolGuid,
                  GvtGopPrivate->GopDevPath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't install protocol gEfiDevicePathProtocolGuid, status %d\n",
      Status
      );
    goto Free;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &GvtGopPrivate->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &GvtGopPrivate->GraphicsOutputProtocol,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Can't install protocol gEfiGraphicsOutputProtocolGuid, status %d\n",
      Status
      );
    goto Free;
  }

  Status = gBS->OpenProtocol (
                ControllerHandle,
                &gEfiPciIoProtocolGuid,
                (VOID **) &ChildPciIo,
                This->DriverBindingHandle,
                GvtGopPrivate->Handle,
                EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                );
  if (EFI_ERROR (Status)) {
    goto Free;
  }

  goto Done;

Free:
  if (GvtGopPrivate->PciIo) {
    if (GvtGopPrivate->OriginalPciAttr) {
      GvtGopPrivate->PciIo->Attributes (
                              GvtGopPrivate->PciIo,
                              EfiPciIoAttributeOperationEnable,
                              GvtGopPrivate->OriginalPciAttr,
                              NULL
                              );
    }
    gBS->CloseProtocol (
           ControllerHandle,
           &gEfiPciIoProtocolGuid,
           This->DriverBindingHandle,
           ControllerHandle
           );
    GvtGopPrivate->PciIo = NULL;
  }

  if (GvtGopPrivate->VirtualGpu) {
    gBS->FreePool (GvtGopPrivate->VirtualGpu);
    GvtGopPrivate->VirtualGpu = NULL;
  }

  if (GvtGopPrivate) {
    gBS->FreePool (GvtGopPrivate);
  }

  if (GvtGopPrivate->GopDevPath) {
    FreePool (GvtGopPrivate->GopDevPath);
    GvtGopPrivate->GopDevPath = NULL;
  }

Done:

  gBS->RestoreTPL (OriginalTPL);

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}


STATIC
EFI_STATUS
EFIAPI
GvtGopBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  ControllerHandle,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer OPTIONAL
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutputProtocol;
  EFI_STATUS                   Status;
  GVT_GOP_PRIVATE_DATA         *GvtGopPrivate = NULL;

  GVT_DEBUG (EFI_D_INFO, "%a: >>>\n", __FUNCTION__);

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **)&GraphicsOutputProtocol,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    Status = EFI_NOT_STARTED;
    goto Done;
  }

  GvtGopPrivate = GVT_GOP_PRIVATE_DATA_FROM_THIS (GraphicsOutputProtocol);
  if (!GvtGopPrivate) {
    Status = EFI_NOT_STARTED;
    GVT_DEBUG (EFI_D_ERROR,
      "Intel GVT-g GOP isn't started, status %d\n", Status
      );
    goto Done;
  }

  gBS->UninstallMultipleProtocolInterfaces (
         GvtGopPrivate->Handle,
         &gEfiGraphicsOutputProtocolGuid,
         &GvtGopPrivate->GraphicsOutputProtocol,
         NULL
         );

  if (GvtGopPrivate->PciIo) {
    gBS->CloseProtocol (
           ControllerHandle,
           &gEfiPciIoProtocolGuid,
           This->DriverBindingHandle,
           ControllerHandle
           );
    GvtGopPrivate->PciIo = NULL;
  }

  Status = IntelVirtualGpuClean (GvtGopPrivate);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR, "Fail to clean virtual GPU, status %d\n", Status);
    goto Done;
  }

  if (GvtGopPrivate->VirtualGpu) {
    gBS->FreePool (GvtGopPrivate->VirtualGpu);
    GvtGopPrivate->VirtualGpu = NULL;
  }

  if (GvtGopPrivate) {
    gBS->FreePool (GvtGopPrivate);
  }

Done:

  GVT_DEBUG (EFI_D_INFO, "%a: <<<\n", __FUNCTION__);

  return Status;
}

STATIC EFI_DRIVER_BINDING_PROTOCOL gGvtGopDriverBinding = {
  GvtGopBindingSupported,
  GvtGopBindingStart,
  GvtGopBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_STATUS
EFIAPI
GvtGopComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
{
  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           gGvtGopDriverNameTable,
           DriverName,
           (BOOLEAN)(This == &gGvtGopDriverComponentName)
           );
}

EFI_STATUS
EFIAPI
GvtGopComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL *This,
  IN  EFI_HANDLE                  ControllerHandle,
  IN  EFI_HANDLE                  ChildHandle OPTIONAL,
  IN  CHAR8                       *Language,
  OUT CHAR16                      **ControllerName
  )
{
  EFI_STATUS Status;

  if (ChildHandle != NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = EfiTestManagedDevice (
             ControllerHandle,
             gGvtGopDriverBinding.DriverBindingHandle,
             &gEfiPciIoProtocolGuid
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           gGvtGopControllerNameTable,
           ControllerName,
           (BOOLEAN)(This == &gGvtGopDriverComponentName)
           );
}

EFI_STATUS
EFIAPI
GvtGopEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gGvtGopDriverBinding,
             ImageHandle,
             &gGvtGopDriverComponentName,
             &gGvtGopDriverComponentName2);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR, "Failed to install driver %d : %d\n", Status);
  }

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}
