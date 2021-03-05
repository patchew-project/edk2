/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <Library/FrameBufferBltLib.h>

typedef struct _BLT_RECTANGLE {
  UINTN X;
  UINTN Y;
  UINTN Width;
  UINTN Height;
} BLT_RECTANGLE, *PBLT_RECTANGLE;

#define DISPLAY_WIDTH_MAX      1920
#define DISPLAY_HEIGHT_MAX     1080
#define DISPLAY_WIDTH_DEFAULT  1024
#define DISPLAY_HEIGHT_DEFAULT 768
#define DISPLAY_MODE_INVALID   0xFFFF

#define DISPLAY_USE_INTERNAL_BLT 1

typedef struct _INTEL_VIRTUAL_GPU_DISPLAY {
  UINTN                                HActive;
  UINTN                                VActive;
  UINTN                                Width;
  UINTN                                Height;
  UINTN                                WidthBytes;
  UINTN                                StrideBytes;
  EFI_GRAPHICS_PIXEL_FORMAT            Format;
  UINTN                                Bpp;
  UINTN                                MaxMode;
  UINTN                                CurrentMode;
  UINTN                                FbSize;
  UINTN                                Pages;
  EFI_PHYSICAL_ADDRESS                 FbGMAddr;
  EFI_PHYSICAL_ADDRESS                 FbPhysicalAddr;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeList;
  FRAME_BUFFER_CONFIGURE               *FrameBufferBltConfigure;
  UINTN                                FrameBufferBltConfigureSize;
} INTEL_VIRTUAL_GPU_DISPLAY, *PINTEL_VIRTUAL_GPU_DISPLAY;

EFI_STATUS
IntelVirtualGpuDisplayInit (
  IN OUT GVT_GOP_PRIVATE_DATA *Private
  );

EFI_STATUS
IntelVirtualGpuDisplayClean (
  IN OUT GVT_GOP_PRIVATE_DATA *Private
  );

EFI_STATUS
EFIAPI
IntelVirtualGpuQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL         *This,
  IN  UINT32                               ModeNumber,
  OUT UINTN                                *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info
  );

EFI_STATUS
EFIAPI
IntelVirtualGpuSetMode (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
  IN UINT32                       ModeNumber
  );

EFI_STATUS
EFIAPI
IntelVirtualGpuBlt (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL      *This,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL     *BltBuffer,   OPTIONAL
  IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
  IN UINTN                             SourceX,
  IN UINTN                             SourceY,
  IN UINTN                             DestinationX,
  IN UINTN                             DestinationY,
  IN UINTN                             Width,
  IN UINTN                             Height,
  IN UINTN                             Delta         OPTIONAL
  );

EFI_STATUS
IntelVirtualGpuEnableDisplay (
  IN OUT GVT_GOP_PRIVATE_DATA *Private,
  IN     UINT32               ModeNumber,
  IN     BOOLEAN              Enable
  );

EFI_STATUS
IntelVirtualGpuNotifyDisplayReady (
  IN GVT_GOP_PRIVATE_DATA *Private,
  IN BOOLEAN              Ready
  );

EFI_STATUS
IntelVirtualGpuBltVideoFill (
  IN PINTEL_VIRTUAL_GPU_DISPLAY          Display,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION BltPixel,
  IN BLT_RECTANGLE                       Destination
  );

EFI_STATUS
IntelVirtualGpuBltVideoToBuffer (
  IN PINTEL_VIRTUAL_GPU_DISPLAY    Display,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,
  IN BLT_RECTANGLE                 Source,
  IN BLT_RECTANGLE                 Destination,
  IN UINTN                         Delta
  );

EFI_STATUS
IntelVirtualGpuBltVideoFromBuffer (
  IN PINTEL_VIRTUAL_GPU_DISPLAY    Display,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,
  IN BLT_RECTANGLE                 Source,
  IN BLT_RECTANGLE                 Destination,
  IN UINTN                         Delta
  );

EFI_STATUS
IntelVirtualGpuBltVideoToVideo (
  IN PINTEL_VIRTUAL_GPU_DISPLAY Display,
  IN BLT_RECTANGLE              Source,
  IN BLT_RECTANGLE              Destination
  );
VOID
InstallVbeShim (
  IN CONST CHAR16         *CardName,
  IN EFI_PHYSICAL_ADDRESS FrameBufferBase
  );

#endif //__DISPLAY_H_
