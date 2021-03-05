/** @file
  Component name for the QEMU video controller.

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __GPUREG_H_
#define __GPUREG_H_

#include "Common.h"

#define MMIO_SIZE 0x200000

#define VGT_PVINFO_PAGE 0x78000
#define VGT_PVINFO_SIZE 0x1000

#define VGT_MAGIC         0x4776544776544776ULL  /* 'vGTvGTvG' */
#define VGT_VERSION_MAJOR 1
#define VGT_VERSION_MINOR 0

#define VGT_DRV_DISPLAY_NOT_READY 0
#define VGT_DRV_DISPLAY_READY     1

struct vgt_if {
  UINT64 magic;    /* VGT_MAGIC */
  UINT16 version_major;
  UINT16 version_minor;
  UINT32 vgt_id;    /* ID of vGT instance */
  UINT32 vgt_caps;    /* VGT capabilities */
  UINT32 rsv1[11];    /* pad to offset 0x40 */
  /*
   *  Data structure to describe the balooning info of resources.
   *  Each VM can only have one portion of continuous area for now.
   *  (May support scattered resource in future)
   *  (starting from offset 0x40)
   */
  struct {
    /* Aperture register balooning */
    struct {
      UINT32 base;
      UINT32 size;
    } mappable_gmadr;  /* aperture */
    /* GMADR register balooning */
    struct {
      UINT32 base;
      UINT32 size;
    } nonmappable_gmadr;  /* non aperture */
    /* allowed fence registers */
    UINT32 fence_num;
    UINT32 rsv2[3];
  } avail_rs;    /* available/assigned resource */
  UINT32 rsv3[0x200 - 24];  /* pad to half page */
  /*
   * The bottom half page is for response from Gfx driver to hypervisor.
   */
  UINT32 rsv4;
  UINT32 display_ready;  /* ready for display owner switch */

  UINT32 rsv5[4];

  UINT32 g2v_notify;
  UINT32 rsv6[5];

  UINT32 cursor_x_hot;
  UINT32 cursor_y_hot;

  struct {
    UINT32 lo;
    UINT32 hi;
  } pdp[4];

  UINT32 execlist_context_descriptor_lo;
  UINT32 execlist_context_descriptor_hi;

  UINT32  rsv7[0x200 - 24];    /* pad to one page */
} PACKED;

#define vgtif_offset(x) (OFFSET_OF(struct vgt_if, x))
#define vgtif_reg(x) (VGT_PVINFO_PAGE + vgtif_offset(x))

typedef enum _GPU_DISPLAY_PIPE {
  PIPE_INVALID = -1,
  PIPE_A = 0,
  PIPE_B,
  PIPE_C,
  PIPE_MAX = PIPE_C
} GPU_DISPLAY_PIPE;

typedef enum _GPU_DISPLAY_PLANE {
  PLANE_PRIMARY = 0,
  PLANE_SPRITE0,
  PLANE_SPRITE1,
  PLANE_MAX,
} GPU_DISPLAY_PLANE;

#define _TRANS_HTOTAL_A 0x60000
#define _TRANS_VTOTAL_A 0x6000C
#define _TRANS_REG_OFFSET(trans) (trans * 0x1000)

#define _PS_WIN_POS_1_A 0x68170
#define _PS_WIN_SZ_1_A 0x68174
#define _PS_CTRL_1_A 0x68180
#define _PS_REG_OFFSET(pipe, id) (pipe * 0x800 + id * 0x100)
#define PS_WIN_POS(pipe, id) (_PS_WIN_POS_1_A + _PS_REG_OFFSET(pipe, id))
#define PS_WIN_SZ(pipe, id) (_PS_WIN_SZ_1_A + _PS_REG_OFFSET(pipe, id))
#define PS_CTRL(pipe, id) (_PS_CTRL_1_A + _PS_REG_OFFSET(pipe, id))
#define   PS_CTRL_SCALER_EN (1 << 31)
#define   PS_CTRL_SCALER_MODE_MASK (0x3 << 28)
#define   PS_CTRL_SCALER_MODE_DYN  (0 << 28)
#define   PS_CTRL_SCALER_MODE_HQ  (1 << 28)
#define   PS_CTRL_SCALER_BINDING_MASK  (0x7 << 25)
#define   PS_CTRL_SCALER_BINDING_PIPE  (0 << 25)
#define   PS_CTRL_PLANE_SEL(plane) (((plane) + 1) << 25)
#define   PS_CTRL_SCALER_FILTER_MASK         (3 << 23)
#define   PS_CTRL_SCALER_FILTER_MEDIUM       (0 << 23)

#define PIPE_REG_OFFSET(pipe) (pipe * 0x1000)
#define _PIPE_CONF_A 0x70008
#define   PIPE_CONF_ENABLE (1 << 31)
#define _PIPE_SRCSZ_A 0x6001C
#define PIPE_CONF(pipe) (_PIPE_CONF_A + PIPE_REG_OFFSET(pipe))
#define PIPESRC(pipe) (_PIPE_SRCSZ_A + PIPE_REG_OFFSET(pipe))

#define _PLANE_CTL_1_A 0x70180
#define   PLANE_CTL_ENABLE (1 << 31)
#define   PLANE_CTL_PIPE_GAMMA_ENABLE (1 << 30)
#define   PLANE_CTL_FORMAT_MASK (0xF << 24)
#define   PLANE_CTL_FORMAT_XRGB_8888 (0x4 << 24)
#define   PLANE_CTL_PIPE_CSC_ENABLE (1 << 23)
#define   PLANE_CTL_KEY_ENABLE_MASK (0x3 << 21)
#define   PLANE_CTL_ORDER_RGBX (1 << 20)
#define   PLANE_CTL_RENDER_DECOMPRESSION_ENABLE (1 << 15)
#define   PLANE_CTL_PLANE_GAMMA_DISABLE (1 << 13)
#define   PLANE_CTL_TILED_MASK (0x7 << 10)
#define   PLANE_CTL_TILED_LINEAR (0 << 10)
#define   PLANE_CTL_ASYNC_FLIP (1 << 9)
#define   PLANE_CTL_ALPHA_MASK (0x3 << 4)
#define   PLANE_CTL_ALPHA_DISABLE (0 << 4)
#define   PLANE_CTL_ROTATE_MASK (0x3 << 0)
#define   PLANE_CTL_ROTATE_0 (0x0 << 0)

#define _PLANE_STRIDE_1_A 0x70188
#define   PLANE_STRIDE_MASK 0x1FF
#define _PLANE_POS_1_A  0x7018C
#define _PLANE_SIZE_1_A 0x70190
#define _PLANE_SURF_1_A 0x7019C
#define _PLANE_REG_OFFSET(pipe, plane) (pipe * 0x1000 + plane * 0x100)

#define HTOTAL(trans) (_TRANS_HTOTAL_A + _TRANS_REG_OFFSET(trans))
#define VTOTAL(trans) (_TRANS_VTOTAL_A + _TRANS_REG_OFFSET(trans))

#define PLANE_CTL(pipe, plane) (_PLANE_CTL_1_A + _PLANE_REG_OFFSET(pipe, plane))
#define PLANE_STRIDE(pipe, plane) (_PLANE_STRIDE_1_A + _PLANE_REG_OFFSET(pipe, plane))
#define PLANE_POS(pipe, plane) (_PLANE_POS_1_A + _PLANE_REG_OFFSET(pipe, plane))
#define PLANE_SIZE(pipe, plane) (_PLANE_SIZE_1_A + _PLANE_REG_OFFSET(pipe, plane))
#define PLANE_SURF(pipe, plane) (_PLANE_SURF_1_A + _PLANE_REG_OFFSET(pipe, plane))

EFI_STATUS
RegRead32 (
  IN  GVT_GOP_PRIVATE_DATA *Private,
  IN  UINT32 Offset,
  OUT UINT32 *ValuePtr
  );

EFI_STATUS
RegWrite32 (
  IN GVT_GOP_PRIVATE_DATA *Private,
  IN UINT32 Offset,
  IN UINT32 Value
  );

#endif //__GPUREG_H_
