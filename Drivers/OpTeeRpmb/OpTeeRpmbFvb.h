/** @file

  Copyright (c) 2020, Linaro Ltd. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __OPTEE_RPMB_FV_
#define __OPTEE_RPMB_FV_

/* SVC Args */
#define SP_SVC_RPMB_READ                0xC4000066
#define SP_SVC_RPMB_WRITE               0xC4000067
#define SP_SVC_GET_UART                 0xC4000068

#define FILENAME "EFI_VARS"

#define NBLOCKS                    (3 * 16) // EFI Vars, FTW working, FTW spare
#define BLOCK_SIZE                 SIZE_4KB
#define FLASH_SIGNATURE            SIGNATURE_32('r', 'p', 'm', 'b')
#define INSTANCE_FROM_FVB_THIS(a)  CR(a, MEM_INSTANCE, FvbProtocol, \
                                      FLASH_SIGNATURE)
enum _RPMB_FILE_MAP {
  EFI_VARS,
  FTW_WORK,
  FTW_SPARE,
};

typedef enum _RPMB_FILE_MAP RPMB_FILE_MAP;

struct _MAP_VAL_TO_FILE {
  CHAR8         *Filename;
  RPMB_FILE_MAP Map;
};

typedef struct _MAP_VAL_TO_FILE MAP_VAL_TO_FILE;

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
