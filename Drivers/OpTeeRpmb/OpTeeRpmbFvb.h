/** @file

  Copyright (c) 2020, Linaro Ltd. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __OPTEE_RPMB_FV_
#define __OPTEE_RPMB_FV_

/* SVC Args */
#define SP_SVC_RPMB_READ                0xC4000066
#define SP_SVC_RPMB_WRITE               0xC4000067

#define FILENAME "EFI_VARS"

#define FLASH_SIGNATURE            SIGNATURE_32('r', 'p', 'm', 'b')
#define INSTANCE_FROM_FVB_THIS(a)  CR(a, MEM_INSTANCE, FvbProtocol, \
                                      FLASH_SIGNATURE)

typedef struct _MEM_INSTANCE         MEM_INSTANCE;
typedef EFI_STATUS (*MEM_INITIALIZE) (MEM_INSTANCE* Instance);
struct _MEM_INSTANCE
{
    UINT32                              Signature;
    MEM_INITIALIZE                      Initialize;
    BOOLEAN                             Initialized;
    EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  FvbProtocol;
    EFI_HANDLE                          Handle;
    EFI_PHYSICAL_ADDRESS                MemBaseAddress;
    UINT16                              BlockSize;
    UINT16                              NBlocks;
};

#endif
