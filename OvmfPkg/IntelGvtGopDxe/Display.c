/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Common.h"
#include "Display.h"
#include "GpuReg.h"
#include "Gtt.h"
#include "VirtualGpu.h"

EFI_STATUS
IntelVirtualGpuDisplayInit (
  IN OUT GVT_GOP_PRIVATE_DATA *Private
  )
{
  EFI_STATUS                 Status = EFI_UNSUPPORTED;
  PINTEL_VIRTUAL_GPU         VirtualGpu;
  PINTEL_VIRTUAL_GPU_DISPLAY Display;
  UINT32                     Val32;
  UINTN                      Width, Height, ModeNumber;
  EFI_TPL                    OriginalTPL;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  VirtualGpu = (PINTEL_VIRTUAL_GPU)Private->VirtualGpu;
  Display = &VirtualGpu->Display;

  /*
   * If PcdVideoHorizontalResolution or PcdVideoVerticalResolution is not set,
   *   GOP will query the mode list reported to find the highest resolution.
   *   Otherwise, check if the set PcdVideo*Resolution is defined.
   *   If not supported, try 800x600 which is required by UEFI/EFI spec.
   *   If still not supported, use the 1st mode in mode list.
   * If there are multiple video devices, graphic console driver will set all
   *   the video devices to the same mode.
   * According to UEFI/EFI spec, in addition to platform design guide, on-board
   *   graphics should support native mode of the display, plug-in graphics
   *   should support 800x600x32 or 640x480x32.
   * According to some OS requirement (i.e. UEFI requirment for Windows 10),
   *   integrated displays should support panel native resolution and external
   *   displays should support the maximum resolution of both GPU and display in
   *   GOP. For alternate display output, it should support native or highest
   *   compatible resolution, otherwise support an known mode to be compatible
   *   with as many monitors as possible (640x480, 1024x768).
   * Due to above requirement, use native resolution if PcdVideo*Resolution is
   *   not defined. To reduce GGTT write overhead, also limit the maximum to
   *   DISPLAY_WIDTH_MAX/DISPLAY_HEIGHT_MAX.
   */

  RegRead32 (Private, HTOTAL(PIPE_A), &Val32);
  Display->HActive = (Val32 & 0xFFF) + 1;
  RegRead32 (Private, VTOTAL(PIPE_A), &Val32);
  Display->VActive = (Val32 & 0xFFF) + 1;

  if (Display->HActive != 0 && Display->VActive != 0) {
    Width = Display->HActive;
    Height = Display->VActive;
    if (Display->HActive > DISPLAY_WIDTH_MAX ||
        Display->VActive > DISPLAY_HEIGHT_MAX) {
      Width = DISPLAY_WIDTH_MAX;
      Height = DISPLAY_HEIGHT_MAX;
    }
  } else {
    Width = DISPLAY_WIDTH_DEFAULT;
    Height = DISPLAY_HEIGHT_DEFAULT;
  }

  Display->Width = Width;
  Display->Height = Height;
  Display->Format = PixelBlueGreenRedReserved8BitPerColor;
  Display->Bpp = 4;
  Display->MaxMode = 1;

  // Add default if defined
  if (PcdGet32 (PcdVideoHorizontalResolution) != 0 &&
      PcdGet32 (PcdVideoVerticalResolution) != 0 &&
      PcdGet32 (PcdVideoHorizontalResolution) != Width &&
      PcdGet32 (PcdVideoVerticalResolution) != Height) {
      ++Display->MaxMode;
  }

  Display->CurrentMode = DISPLAY_MODE_INVALID;
  Display->FrameBufferBltConfigure = NULL;
  Display->FrameBufferBltConfigureSize = 0;

  // Linear must start at 256K, stride align at 64
  Display->WidthBytes = Display->Width * Display->Bpp;
  Display->StrideBytes = ALIGN_VALUE (Display->WidthBytes, 64);
  Display->FbSize = Display->StrideBytes * Display->Height;
  Display->Pages = EFI_SIZE_TO_PAGES (Display->FbSize);

  Display->FbGMAddr = VirtualGpu->GpuMemAddr + VirtualGpu->VisibleOffset;
  Display->FbGMAddr = ALIGN_VALUE (Display->FbGMAddr, SIZE_256KB);

  Status = gBS->AllocatePages (
                AllocateAnyPages,
                EfiReservedMemoryType,
                Display->Pages,
                &Display->FbPhysicalAddr
                );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "AllocatePages failed for display FB, pages %d, size %lx, status %d\n",
      Display->Pages, Display->FbSize, Status
      );
    return Status;
  }

  Status = UpdateGGTT (Private,
             Display->FbGMAddr,
             Display->FbPhysicalAddr,
             Display->Pages
             );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Fail to Update GGTT for display, status %d\n", Status
      );
    goto Done;
  }

  OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);
  Status = IntelVirtualGpuBltVideoFill (
             Display,
             (EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION){{0, 0, 0, 0}},
             (BLT_RECTANGLE){0, 0, Display->Width, Display->Height});
  gBS->RestoreTPL (OriginalTPL);
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "Fail to clear rectangle at [%d, %d] size %dx%d with color 0x%08x, status %d\n",
      (BLT_RECTANGLE){0, 0, Display->Width, Display->Height}.X,
      (BLT_RECTANGLE){0, 0, Display->Width, Display->Height}.Y,
      (BLT_RECTANGLE){0, 0, Display->Width, Display->Height}.Width,
      (BLT_RECTANGLE){0, 0, Display->Width, Display->Height}.Height,
      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION){{0, 0, 0, 0}}.Raw,
      Status
      );
    goto Done;
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) * Display->MaxMode,
                  (VOID **)&Display->ModeList
                  );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "AllocatePool failed for display mode list, size %d, status %d\n",
      sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) * Display->MaxMode,
      Status
      );
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  for (ModeNumber = 0; ModeNumber < Display->MaxMode; ModeNumber++) {
    Display->ModeList[ModeNumber].Version = 0;
    Display->ModeList[ModeNumber].HorizontalResolution = Display->Width;
    Display->ModeList[ModeNumber].VerticalResolution = Display->Height;
    Display->ModeList[ModeNumber].PixelFormat = Display->Format;
    Display->ModeList[ModeNumber].PixelsPerScanLine = Display->Width;
  }
  if (Display->MaxMode > 1) {
    Display->ModeList[1].HorizontalResolution = PcdGet32 (PcdVideoHorizontalResolution);
    Display->ModeList[1].VerticalResolution = PcdGet32 (PcdVideoVerticalResolution);
    Display->ModeList[1].PixelsPerScanLine = PcdGet32 (PcdVideoHorizontalResolution);
  }

  Private->GraphicsOutputProtocol.QueryMode = IntelVirtualGpuQueryMode;
  Private->GraphicsOutputProtocol.SetMode = IntelVirtualGpuSetMode;
  Private->GraphicsOutputProtocol.Blt = IntelVirtualGpuBlt;
  Private->GraphicsOutputProtocol.Mode->MaxMode = Display->MaxMode;
  Private->GraphicsOutputProtocol.Mode->Mode = Display->CurrentMode;
  Private->GraphicsOutputProtocol.Mode->SizeOfInfo =
    sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) * Display->MaxMode;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  Private->GraphicsOutputProtocol.Mode->SizeOfInfo,
                  (VOID **)&Private->GraphicsOutputProtocol.Mode->Info
                  );
  if (EFI_ERROR (Status)) {
    GVT_DEBUG (EFI_D_ERROR,
      "AllocatePool failed for display mode info, size %d, status %d\n",
      Private->GraphicsOutputProtocol.Mode->SizeOfInfo, Status
      );
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  CopyMem (
    Private->GraphicsOutputProtocol.Mode->Info,
    Display->ModeList,
    sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) * Display->MaxMode
    );

  Private->GraphicsOutputProtocol.Mode->FrameBufferBase = Display->FbGMAddr;
  Private->GraphicsOutputProtocol.Mode->FrameBufferSize = Display->FbSize;

  InstallVbeShim (L"GVT-g VBIOS", Display->FbGMAddr);

  GVT_DEBUG (EFI_D_INFO,
    "modes %d, max %dx%d, OVMF default %dx%d\n",
    Display->MaxMode,
    Display->Width, Display->Height,
    PcdGet32 (PcdVideoHorizontalResolution),
    PcdGet32 (PcdVideoVerticalResolution)
    );
  for (ModeNumber = 0; ModeNumber < Display->MaxMode; ModeNumber++) {
    GVT_DEBUG (EFI_D_INFO,
      "  mode %d: %dx%d BGRX, stride %d\n",
      ModeNumber,
      Display->ModeList[ModeNumber].HorizontalResolution,
      Display->ModeList[ModeNumber].VerticalResolution,
      ALIGN_VALUE (Display->ModeList[ModeNumber].HorizontalResolution * Display->Bpp, 64)
      );
  }
  GVT_DEBUG (EFI_D_INFO,
    "FrameBuffer: GMADR %lx, PADDR %lx, size %lx, pages %d, INTERNAL_BLT %d\n",
    Display->FbGMAddr, Display->FbPhysicalAddr, Display->FbSize, Display->Pages,
    DISPLAY_USE_INTERNAL_BLT
    );

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;

}

EFI_STATUS
IntelVirtualGpuDisplayClean (
  IN OUT GVT_GOP_PRIVATE_DATA *Private
  )
{
  EFI_STATUS                 Status = EFI_INVALID_PARAMETER;
  PINTEL_VIRTUAL_GPU         VirtualGpu;
  PINTEL_VIRTUAL_GPU_DISPLAY Display;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  VirtualGpu = (PINTEL_VIRTUAL_GPU)Private->VirtualGpu;
  Display = &VirtualGpu->Display;

  if (Private->GraphicsOutputProtocol.Mode->Info) {
    Status = gBS->FreePool (Private->GraphicsOutputProtocol.Mode->Info);
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "FreePool failed for display mode info, size %d, status %d\n",
        Private->GraphicsOutputProtocol.Mode->SizeOfInfo, Status
        );
        goto Done;
    }
    Private->GraphicsOutputProtocol.Mode->SizeOfInfo = 0;
    Private->GraphicsOutputProtocol.Mode->Info = NULL;
  }
  Private->GraphicsOutputProtocol.Mode->MaxMode = 0;
  Private->GraphicsOutputProtocol.Mode->Mode = DISPLAY_MODE_INVALID;

  if (Display->FbPhysicalAddr) {
    Status = gBS->FreePages (Display->FbPhysicalAddr, Display->Pages);
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "FreePages failed for display FB, pages %d, size %lx, status %d\n",
        Display->Pages, Display->FbSize, Status
        );
        goto Done;
    }
    Display->FbPhysicalAddr = 0;
    Display->Pages = 0;
    Display->FbSize = 0;
  }

  Status = EFI_SUCCESS;

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
EFIAPI
IntelVirtualGpuQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL         *This,
  IN  UINT32                               ModeNumber,
  OUT UINTN                                *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info
  )
{
  EFI_STATUS                 Status = EFI_UNSUPPORTED;
  GVT_GOP_PRIVATE_DATA       *GvtGopPrivate = NULL;
  PINTEL_VIRTUAL_GPU         VirtualGpu;
  PINTEL_VIRTUAL_GPU_DISPLAY Display;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  GvtGopPrivate = GVT_GOP_PRIVATE_DATA_FROM_THIS (This);
  VirtualGpu = (PINTEL_VIRTUAL_GPU)GvtGopPrivate->VirtualGpu;
  Display = &VirtualGpu->Display;

  if (ModeNumber >= Display->MaxMode) {
    GVT_DEBUG (EFI_D_ERROR,
      "Invalid ModeNumber, request %d, max %d, status %d\n",
      ModeNumber, Display->MaxMode, Status
      );
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
                  (VOID **)Info
                  );
  if (EFI_ERROR (Status) || *Info == NULL) {
    GVT_DEBUG (EFI_D_ERROR,
      "AllocatePool failed for queried mode info, size %d, status %d\n",
      sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION), Status
      );
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  *SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  CopyMem (
    *Info,
    &Display->ModeList[ModeNumber],
    sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION)
    );

  Status = EFI_SUCCESS;

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
EFIAPI
IntelVirtualGpuSetMode (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
  IN UINT32                       ModeNumber
  )
{
  EFI_STATUS                          Status = EFI_UNSUPPORTED;
  GVT_GOP_PRIVATE_DATA                *GvtGopPrivate = NULL;
  PINTEL_VIRTUAL_GPU                  VirtualGpu;
  PINTEL_VIRTUAL_GPU_DISPLAY          Display;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  GvtGopPrivate = GVT_GOP_PRIVATE_DATA_FROM_THIS (This);
  VirtualGpu = (PINTEL_VIRTUAL_GPU)GvtGopPrivate->VirtualGpu;
  Display = &VirtualGpu->Display;

  if (ModeNumber >= Display->MaxMode) {
    GVT_DEBUG (EFI_D_ERROR,
      "Invalid ModeNumber, request %d, max %d, status %d\n",
      ModeNumber, Display->MaxMode, Status
      );
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

#if (DISPLAY_USE_INTERNAL_BLT == 1)
  Status = IntelVirtualGpuBltVideoFill (
             Display,
             (EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION){{0, 0, 0, 0}},
             (BLT_RECTANGLE){0, 0, This->Mode->Info->HorizontalResolution, This->Mode->Info->VerticalResolution});
  if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "IntelVirtualGpuBltVideoFill failed for mode %d, status %d\n",
        ModeNumber,
        Status
        );
    }
#else
  Status = FrameBufferBltConfigure (
             (VOID*) (UINTN) This->Mode->FrameBufferBase,
             This->Mode->Info,
             Display->FrameBufferBltConfigure,
             &Display->FrameBufferBltConfigureSize
             );
  if (Status == RETURN_BUFFER_TOO_SMALL) {
    if (Display->FrameBufferBltConfigure != NULL) {
      Status = gBS->FreePool (Display->FrameBufferBltConfigure);
      if (EFI_ERROR (Status)) {
        GVT_DEBUG (EFI_D_ERROR,
          "FreePool failed for FrameBufferBltConfigure, status %d\n",
          Status
          );
          goto Done;
      }
    }
    Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  Display->FrameBufferBltConfigureSize,
                  (VOID **)&Display->FrameBufferBltConfigure
                  );
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "AllocatePool failed for FrameBufferBltConfigure, size %d, status %d\n",
        Display->FrameBufferBltConfigureSize,
        Status
        );
      goto Done;
    }

    Status = FrameBufferBltConfigure (
                (VOID*) (UINTN) This->Mode->FrameBufferBase,
                This->Mode->Info,
                Display->FrameBufferBltConfigure,
                &Display->FrameBufferBltConfigureSize
                );
    if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "FrameBufferBltConfigure failed for mode %d, status %d\n",
        ModeNumber,
        Status
        );
      goto Done;
    }
  }

  Status = FrameBufferBlt (
             Display->FrameBufferBltConfigure,
             &(EFI_GRAPHICS_OUTPUT_BLT_PIXEL){0, 0, 0, 0},
             EfiBltVideoFill,
             0, 0,
             0, 0,
             This->Mode->Info->HorizontalResolution,
             This->Mode->Info->VerticalResolution,
             0
             );
  if (EFI_ERROR (Status)) {
      GVT_DEBUG (EFI_D_ERROR,
        "FrameBufferBlt BltOperation %d failed for mode %d, color 0x%08x, status %d\n",
        EfiBltVideoFill,
        ModeNumber,
        (EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION){{0, 0, 0, 0}}.Raw,
        Status
        );
    }
#endif

  Status = IntelVirtualGpuEnableDisplay (
             GvtGopPrivate,
             ModeNumber,
             FALSE
             );

  Status = IntelVirtualGpuEnableDisplay (
             GvtGopPrivate,
             ModeNumber,
             TRUE
             );

  // Set current mode info in GOP
  This->Mode->Mode = ModeNumber;
  CopyMem (
    This->Mode->Info,
    &Display->ModeList[ModeNumber],
    sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION)
    );

  GVT_DEBUG (EFI_D_INFO, "Set mode %d, %dx%d, status %d\n",
    ModeNumber,
    Display->ModeList[ModeNumber].HorizontalResolution,
    Display->ModeList[ModeNumber].VerticalResolution,
    Status
    );

Done:
  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
EFIAPI
IntelVirtualGpuBlt (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL      *This,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL     *BltBuffer, OPTIONAL
  IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
  IN UINTN                             SourceX,
  IN UINTN                             SourceY,
  IN UINTN                             DestinationX,
  IN UINTN                             DestinationY,
  IN UINTN                             Width,
  IN UINTN                             Height,
  IN UINTN                             Delta OPTIONAL
  )
{
  EFI_STATUS                 Status = EFI_UNSUPPORTED;
  GVT_GOP_PRIVATE_DATA       *GvtGopPrivate = NULL;
  PINTEL_VIRTUAL_GPU         VirtualGpu;
  PINTEL_VIRTUAL_GPU_DISPLAY Display;
  EFI_TPL                    OriginalTPL;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  GvtGopPrivate = GVT_GOP_PRIVATE_DATA_FROM_THIS (This);
  VirtualGpu = (PINTEL_VIRTUAL_GPU)GvtGopPrivate->VirtualGpu;
  Display = &VirtualGpu->Display;

  OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);

#if (DISPLAY_USE_INTERNAL_BLT == 1)
  switch (BltOperation) {
  case EfiBltVideoFill:
    Status = IntelVirtualGpuBltVideoFill (
               Display,
               (EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION)(*BltBuffer),
               (BLT_RECTANGLE){DestinationX, DestinationY, Width, Height});
    RegWrite32 (GvtGopPrivate,
      PLANE_SURF(PIPE_A, PLANE_PRIMARY),
      Display->FbGMAddr
      );
    break;
  case EfiBltVideoToBltBuffer:
    Status = IntelVirtualGpuBltVideoToBuffer (
               Display,
               BltBuffer,
               (BLT_RECTANGLE){SourceX, SourceY, Width, Height},
               (BLT_RECTANGLE){DestinationX, DestinationY, Width, Height},
               Delta
               );
    break;
  case EfiBltBufferToVideo:
    Status = IntelVirtualGpuBltVideoFromBuffer (
               Display,
               BltBuffer,
               (BLT_RECTANGLE){SourceX, SourceY, Width, Height},
               (BLT_RECTANGLE){DestinationX, DestinationY, Width, Height},
               Delta
               );
    RegWrite32 (GvtGopPrivate,
      PLANE_SURF(PIPE_A, PLANE_PRIMARY),
      Display->FbGMAddr
      );
    break;
  case EfiBltVideoToVideo:
    Status = IntelVirtualGpuBltVideoToVideo (
               Display,
               (BLT_RECTANGLE){SourceX, SourceY, Width, Height},
               (BLT_RECTANGLE){DestinationX, DestinationY, Width, Height}
               );
    RegWrite32 (GvtGopPrivate,
      PLANE_SURF(PIPE_A, PLANE_PRIMARY),
      Display->FbGMAddr
      );
    break;
  default:
    GVT_DEBUG (EFI_D_INFO, "Unsupported EFI_GRAPHICS_OUTPUT_BLT_OPERATION %d\n", BltOperation);
    Status = EFI_UNSUPPORTED;
    break;
  }
#else
  switch (BltOperation) {
  case EfiBltVideoToBltBuffer:
  case EfiBltVideoFill:
  case EfiBltBufferToVideo:
  case EfiBltVideoToVideo:
    Status = FrameBufferBlt (
               Display->FrameBufferBltConfigure,
               BltBuffer,
               BltOperation,
               SourceX,
               SourceY,
               DestinationX,
               DestinationY,
               Width,
               Height,
               Delta
               );
    if (BltOperation != EfiBltVideoToBltBuffer) {
      RegWrite32 (GvtGopPrivate,
        PLANE_SURF(PIPE_A, PLANE_PRIMARY),
        Display->FbGMAddr
        );
    }
    break;
  default:
    GVT_DEBUG (EFI_D_INFO, "Unsupported EFI_GRAPHICS_OUTPUT_BLT_OPERATION %d\n", BltOperation);
    Status = EFI_UNSUPPORTED;
    break;
  }
#endif

  gBS->RestoreTPL (OriginalTPL);

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
IntelVirtualGpuEnableDisplay (
  IN OUT GVT_GOP_PRIVATE_DATA *Private,
  IN     UINT32               ModeNumber,
  IN     BOOLEAN              Enable
  )
{
  EFI_STATUS                 Status = EFI_INVALID_PARAMETER;
  PINTEL_VIRTUAL_GPU         VirtualGpu;
  PINTEL_VIRTUAL_GPU_DISPLAY Display;
  UINT32                     Width, Height, Val32;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  VirtualGpu = (PINTEL_VIRTUAL_GPU)Private->VirtualGpu;
  Display = &VirtualGpu->Display;

  if (ModeNumber >= Display->MaxMode) {
    GVT_DEBUG (EFI_D_ERROR,
      "Invalid ModeNumber, request %d, max %d, status %d\n",
      ModeNumber, Display->MaxMode, Status
      );
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Width = Display->ModeList[ModeNumber].HorizontalResolution;
  Height = Display->ModeList[ModeNumber].VerticalResolution;

  if (Enable) {
    Display->CurrentMode = ModeNumber;

    Val32 = (Display->HActive - 1) << 16;
    Val32 |= Display->VActive;
    RegWrite32 (Private, PIPESRC(PIPE_A), Val32);

    RegRead32 (Private, PIPE_CONF(PIPE_A), &Val32);
    Val32 |= PIPE_CONF_ENABLE;
    RegWrite32 (Private, PIPE_CONF(PIPE_A), Val32);

    Val32 = (Width - 1) & 0xFFF;
    Val32 |= ((Height - 1) & 0xFFF) << 16;
    RegWrite32 (Private, PLANE_SIZE(PIPE_A, PLANE_PRIMARY), Val32);
    RegWrite32 (Private, PLANE_POS(PIPE_A, PLANE_PRIMARY), 0);

    // Convert mode with to stride in chunks of 64 bytes as required by PLANE_STRIDE
    Val32 = Display->ModeList[ModeNumber].HorizontalResolution * Display->Bpp;
    Val32 = ALIGN_VALUE (Val32, 64);
    Val32 = (Val32 / 64) & PLANE_STRIDE_MASK;
    RegWrite32 (Private, PLANE_STRIDE(PIPE_A, PLANE_PRIMARY), Val32);

    RegWrite32 (Private, PLANE_SURF(PIPE_A, PLANE_PRIMARY), Display->FbGMAddr);

    // Stretch to fullscreen if current mode is smaller than H/V active.
    if (Display->HActive != Width ||
        Display->VActive != Height) {
      RegWrite32 (Private, PS_WIN_POS(PIPE_A, 0), 0);
      RegWrite32 (Private,
        PS_WIN_SZ(PIPE_A, 0),
        Display->HActive << 16 | Display->VActive
        );
      RegRead32 (Private, PS_CTRL(PIPE_A, 0), &Val32);
      Val32 |= PS_CTRL_SCALER_EN;
      Val32 &= ~PS_CTRL_SCALER_MODE_MASK;
      Val32 |= PS_CTRL_SCALER_MODE_DYN;
      Val32 &= ~PS_CTRL_SCALER_BINDING_MASK;
      Val32 |= PS_CTRL_PLANE_SEL(PLANE_PRIMARY);
      Val32 &= ~PS_CTRL_SCALER_FILTER_MASK;
      Val32 |= PS_CTRL_SCALER_FILTER_MEDIUM;
      RegWrite32 (Private, PS_CTRL(PIPE_A, 0), Val32);
    }

    RegRead32 (Private, PLANE_CTL(PIPE_A, PLANE_PRIMARY), &Val32);
    Val32 |= PLANE_CTL_ENABLE;
    Val32 &= ~PLANE_CTL_PIPE_GAMMA_ENABLE;
    Val32 &= ~PLANE_CTL_FORMAT_MASK;
    Val32 |= PLANE_CTL_FORMAT_XRGB_8888;
    Val32 &= ~PLANE_CTL_PIPE_CSC_ENABLE;
    Val32 &= ~PLANE_CTL_KEY_ENABLE_MASK;
    Val32 &= ~PLANE_CTL_ORDER_RGBX;
    if (Display->ModeList[ModeNumber].PixelFormat == PixelRedGreenBlueReserved8BitPerColor) {
      Val32 |= PLANE_CTL_ORDER_RGBX;
    }
    Val32 &= ~PLANE_CTL_RENDER_DECOMPRESSION_ENABLE;
    Val32 |= PLANE_CTL_PLANE_GAMMA_DISABLE;
    Val32 &= ~PLANE_CTL_TILED_MASK;
    Val32 |= PLANE_CTL_TILED_LINEAR;
    Val32 &= ~PLANE_CTL_ASYNC_FLIP;
    Val32 &= ~PLANE_CTL_ALPHA_MASK;
    Val32 |= PLANE_CTL_ALPHA_DISABLE;
    Val32 &= ~PLANE_CTL_ROTATE_MASK;
    Val32 |= PLANE_CTL_ROTATE_0;
    RegWrite32 (Private, PLANE_CTL(PIPE_A, PLANE_PRIMARY), Val32);
  } else {
    Display->CurrentMode = DISPLAY_MODE_INVALID;

    RegRead32 (Private, PLANE_CTL(PIPE_A, PLANE_PRIMARY), &Val32);
    Val32 &= ~PLANE_CTL_ENABLE;
    RegWrite32 (Private, PLANE_CTL(PIPE_A, PLANE_PRIMARY), Val32);
    RegWrite32 (Private, PLANE_SURF(PIPE_A, PLANE_PRIMARY), 0);

    RegRead32 (Private, PS_CTRL(PIPE_A, 0), &Val32);
    Val32 &= ~PS_CTRL_SCALER_EN;
    RegWrite32 (Private, PS_CTRL(PIPE_A, 0), Val32);
    RegWrite32 (Private, PS_WIN_POS(PIPE_A, 0), 0);
    RegWrite32 (Private, PS_WIN_SZ(PIPE_A, 0), 0);

    RegRead32 (Private, PIPE_CONF(PIPE_A), &Val32);
    Val32 &= ~PIPE_CONF_ENABLE;
    RegWrite32 (Private, PIPE_CONF(PIPE_A), Val32);
  }

  Status = EFI_SUCCESS;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: %a mode %dx%d 0x%x, scaling %a\n",
    __FUNCTION__,
    Enable ? "Enable" : "Disable",
    Width,
    Height,
    Display->FbGMAddr,
    (Display->HActive != Width ||
     Display->VActive != Height) ? "On" : "Off");

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
IntelVirtualGpuNotifyDisplayReady (
  IN GVT_GOP_PRIVATE_DATA *Private,
  IN BOOLEAN              Ready
  )
{
  return RegWrite32 (
           Private,
           vgtif_reg(display_ready),
           Ready ? VGT_DRV_DISPLAY_READY : VGT_DRV_DISPLAY_NOT_READY
           );
}

EFI_STATUS
IntelVirtualGpuBltVideoFill (
  IN PINTEL_VIRTUAL_GPU_DISPLAY          Display,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION BltPixel,
  IN BLT_RECTANGLE                       Destination
  )
{
  EFI_STATUS Status = EFI_INVALID_PARAMETER;
  VOID       *DestAddr;
  UINTN      DestBytes, ModeStrideBytes, Line;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  if (Destination.Width == 0 || Destination.Height == 0) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoFill invalid destination rectangle [%d, %d] \n",
      Destination.Width, Destination.Height
      );
    goto Done;
  }

  if ((Destination.X + Destination.Width > Display->Width) ||
      (Destination.Y + Destination.Height > Display->Height)) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoFill destination [%d, %d] to [%d, %d] ouf of range [%d, %d]\n",
      Destination.X, Destination.Y,
      Destination.X + Destination.Width, Destination.Y + Destination.Height,
      Display->Width, Display->Height
      );
    goto Done;
  }

  if (Display->CurrentMode == DISPLAY_MODE_INVALID) {
    ModeStrideBytes = Display->Width;
  } else {
    ModeStrideBytes = Display->ModeList[Display->CurrentMode].HorizontalResolution;
  }
  ModeStrideBytes = ALIGN_VALUE (ModeStrideBytes * Display->Bpp, 64);

  if (Destination.Width * Display->Bpp == ModeStrideBytes) {
    DestAddr = (UINT8*)Display->FbGMAddr + Destination.Y * ModeStrideBytes;
    DestBytes = Destination.Width * Display->Bpp * Destination.Height;
    SetMem32 ((VOID*)DestAddr, DestBytes, BltPixel.Raw);
  } else {

    for (Line = 0; Line < Destination.Height; Line++) {
      DestAddr = (UINT8*)Display->FbGMAddr +
        (Line + Destination.Y) * ModeStrideBytes +
        Destination.X * Display->Bpp;
      DestBytes = Destination.Width * Display->Bpp;

      SetMem32 (DestAddr, DestBytes, BltPixel.Raw);
    }
  }

  Status = EFI_SUCCESS;

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
IntelVirtualGpuBltVideoToBuffer (
  IN PINTEL_VIRTUAL_GPU_DISPLAY    Display,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,
  IN BLT_RECTANGLE                 Source,
  IN BLT_RECTANGLE                 Destination,
  IN UINTN                         Delta
  )
{
  EFI_STATUS Status = EFI_INVALID_PARAMETER;
  VOID       *SourceAddr, *DestAddr;
  UINTN      DestStride, CopyBytes, ModeStrideBytes, Line;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  if (Source.Width == 0 || Source.Height == 0) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoToBltBuffer invalid source rectangle [%d, %d] \n",
      Source.Width, Source.Height
      );
    goto Done;
  }

  if (Source.Width != Destination.Width ||
      Source.Height != Destination.Height) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoToBltBuffer size mismatch: source %dx%d, destination %dx%d\n",
      Source.Width, Source.Height, Destination.Width, Destination.Height
      );
    goto Done;
  }

  if ((Source.X + Source.Width > Display->Width) ||
      (Source.Y + Source.Height > Display->Height)) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoToBltBuffer source [%d, %d] to [%d, %d] ouf of range [%d, %d]\n",
      Source.X, Source.Y,
      Source.X + Source.Width, Source.Y + Source.Height,
      Display->Width, Display->Height
      );
    goto Done;
  }

  if (Destination.X != 0 || Destination.Y != 0) {
    DestStride = Delta;
  } else {
    DestStride = Destination.Width * Display->Bpp;
  }

  if (Display->CurrentMode == DISPLAY_MODE_INVALID) {
    ModeStrideBytes = Display->Width;
  } else {
    ModeStrideBytes = Display->ModeList[Display->CurrentMode].HorizontalResolution;
  }
  ModeStrideBytes = ALIGN_VALUE (ModeStrideBytes * Display->Bpp, 64);

  for (Line = 0; Line < Source.Height; Line++) {
    SourceAddr = (UINT8*)Display->FbGMAddr +
      (Source.Y + Line) * ModeStrideBytes +
      Source.X * Display->Bpp;
    DestAddr = (UINT8*)BltBuffer + (Destination.Y + Line) * DestStride;
    DestAddr = (UINT8*)DestAddr + Destination.X * Display->Bpp;
    CopyBytes = Source.Width * Display->Bpp;
    CopyMem (DestAddr, SourceAddr, CopyBytes);
  }

  GVT_DEBUG (EFI_D_VERBOSE,
    "EfiBltVideoToBltBuffer [%d, %d] >> [%d, %d] size [%d, %d] Delta %d\n",
    Source.X, Source.Y,
    Destination.X, Destination.Y,
    Source.Width, Source.Height,
    Delta
    );

  Status = EFI_SUCCESS;

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
IntelVirtualGpuBltVideoFromBuffer (
  IN PINTEL_VIRTUAL_GPU_DISPLAY    Display,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,
  IN BLT_RECTANGLE                 Source,
  IN BLT_RECTANGLE                 Destination,
  IN UINTN                         Delta
  )
{
  EFI_STATUS Status = EFI_INVALID_PARAMETER;
  VOID       *SourceAddr, *DestAddr;
  UINTN      SourceStride, CopyBytes, ModeStrideBytes, Line;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  if (Source.Width == 0 || Source.Height == 0) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltBufferToVideo invalid source rectangle [%d, %d] \n",
      Source.Width, Source.Height
      );
    goto Done;
  }

  if (Source.Width != Destination.Width || Source.Height != Destination.Height) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltBufferToVideo size mismatch: source %dx%d, destination %dx%d\n",
      Source.Width, Source.Height, Destination.Width, Destination.Height
      );
    goto Done;
  }

  if ((Destination.X + Destination.Width > Display->Width) ||
      (Destination.Y + Destination.Height > Display->Height)) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltBufferToVideo destination [%d, %d] to [%d, %d] ouf of range [%d, %d]\n",
      Destination.X, Destination.Y,
      Destination.X + Destination.Width, Destination.Y + Destination.Height,
      Display->Width, Display->Height
      );
    goto Done;
  }

  if (Source.X != 0 || Source.Y != 0) {
    SourceStride = Delta;
  } else {
    SourceStride = Source.Width * Display->Bpp;
  }

  if (Display->CurrentMode == DISPLAY_MODE_INVALID) {
    ModeStrideBytes = Display->Width;
  } else {
    ModeStrideBytes = Display->ModeList[Display->CurrentMode].HorizontalResolution;
  }
  ModeStrideBytes = ALIGN_VALUE (ModeStrideBytes * Display->Bpp, 64);

  for (Line = 0; Line < Source.Height; Line++) {
    SourceAddr = (UINT8*)BltBuffer +
      (Source.Y + Line) * SourceStride +
      Source.X * Display->Bpp;
    DestAddr = (UINT8*)Display->FbGMAddr +
      (Destination.Y + Line) * ModeStrideBytes +
      Destination.X * Display->Bpp;
    CopyBytes = Source.Width * Display->Bpp;
    CopyMem (DestAddr, SourceAddr, CopyBytes);
  }

  GVT_DEBUG (EFI_D_VERBOSE, "EfiBltBufferToVideo [%d, %d] >> [%d, %d] size [%d, %d] Delta %d\n",
    Source.X, Source.Y,
    Destination.X, Destination.Y,
    Source.Width, Source.Height,
    Delta
    );

  Status = EFI_SUCCESS;

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}

EFI_STATUS
IntelVirtualGpuBltVideoToVideo (
  IN PINTEL_VIRTUAL_GPU_DISPLAY Display,
  IN BLT_RECTANGLE              Source,
  IN BLT_RECTANGLE              Destination
  )
{
  EFI_STATUS Status = EFI_INVALID_PARAMETER;
  VOID       *SourceAddr, *DestAddr;
  UINTN      CopyBytes, ModeStrideBytes, Line;

  GVT_DEBUG (EFI_D_VERBOSE, "%a: >>>\n", __FUNCTION__);

  if (Source.Width == 0 || Source.Height == 0) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoToVideo invalid source rectangle [%d, %d] \n",
      Source.Width, Source.Height
      );
    goto Done;
  }

  if (Source.Width != Destination.Width || Source.Height != Destination.Height) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoToVideo size mismatch: source %dx%d, destination %dx%d\n",
      Source.Width, Source.Height, Destination.Width, Destination.Height
      );
    goto Done;
  }

  if ((Source.X + Source.Width > Display->Width) ||
      (Source.Y + Source.Height > Display->Height)) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoToVideo source [%d, %d] to [%d, %d] ouf of range [%d, %d]\n",
      Source.X, Source.Y,
      Source.X + Source.Width, Source.Y + Source.Height,
      Display->Width, Display->Height
      );
    goto Done;
  }

  if ((Destination.X + Destination.Width > Display->Width) ||
      (Destination.Y + Destination.Height > Display->Height)) {
    GVT_DEBUG (EFI_D_ERROR,
      "EfiBltVideoToVideo destination [%d, %d] to [%d, %d] ouf of range [%d, %d]\n",
      Destination.X, Destination.Y,
      Destination.X + Destination.Width, Destination.Y + Destination.Height,
      Display->Width, Display->Height
      );
    goto Done;
  }

  if (Display->CurrentMode == DISPLAY_MODE_INVALID) {
    ModeStrideBytes = Display->Width;
  } else {
    ModeStrideBytes = Display->ModeList[Display->CurrentMode].HorizontalResolution;
  }
  ModeStrideBytes = ALIGN_VALUE (ModeStrideBytes * Display->Bpp, 64);

  for (Line = 0; Line < Source.Height; Line++) {
    SourceAddr = (UINT8*)Display->FbGMAddr +
      (Source.Y + Line) * ModeStrideBytes +
      Source.X * Display->Bpp;
    DestAddr = (UINT8*)Display->FbGMAddr +
      (Destination.Y + Line)* ModeStrideBytes +
      Destination.X * Display->Bpp;
    CopyBytes = Source.Width * Display->Bpp;
    //
    // Overlap could corrupt source content:
    // src <----|---->
    // dst      <----|---->
    //
    if (DestAddr > SourceAddr && DestAddr < (SourceAddr + CopyBytes)) {
      CopyMem (
        SourceAddr + CopyBytes,
        DestAddr,
        SourceAddr + CopyBytes - DestAddr
        );
      CopyMem (DestAddr, SourceAddr, DestAddr - SourceAddr);
    //
    // Overlap won't corrupt source content:
    // src      <----|---->
    // dst <----|---->
    //
    // No overlap
    // src <--------->
    // dst                         <--------->
    //
    } else {
      CopyMem (DestAddr, SourceAddr, CopyBytes);
    }
  }

  Status = EFI_SUCCESS;

Done:

  GVT_DEBUG (EFI_D_VERBOSE, "%a: <<<\n", __FUNCTION__);

  return Status;
}
