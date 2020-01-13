/** @file
  This library is Unified Hash API. It will redirect hash request to
  the hash handler specified by PcdSystemHashPolicy such as SHA1, SHA256,
  SHA384 and SM3...

Copyright (c) 2013 - 2020, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseHashLib.h>

#include "BaseHashLibCommon.h"

/**
  Init hash sequence.

  @param HashHandle  Hash handle.

  @retval TRUE       Hash start and HashHandle returned.
  @retval FALSE      Hash Init unsuccessful.
**/
BOOLEAN
EFIAPI
HashApiInit (
  OUT  HASH_HANDLE   *HashHandle
)
{
  BOOLEAN     Status;
  UINT8       HashPolicy;
  HASH_HANDLE Handle;

  HashPolicy = PcdGet8 (PcdSystemHashPolicy);

  Status = HashInitInternal (HashPolicy, &Handle);

  *HashHandle = Handle;

  return Status;
}

/**
  Update hash data.

  @param HashHandle    Hash handle.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.

  @retval TRUE         Hash updated.
  @retval FALSE        Hash updated unsuccessful.
**/
BOOLEAN
EFIAPI
HashApiUpdate (
  IN HASH_HANDLE    HashHandle,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
)
{
  BOOLEAN     Status;
  UINT8       HashPolicy;

  HashPolicy = PcdGet8 (PcdSystemHashPolicy);

  Status = HashUpdateInternal (HashPolicy, HashHandle, DataToHash, DataToHashLen);

  return Status;
}

/**
  Hash complete.

  @param HashHandle    Hash handle.
  @param Digest        Hash Digest.

  @retval TRUE         Hash complete and Digest is returned.
  @retval FALSE        Hash complete unsuccessful.
**/
BOOLEAN
EFIAPI
HashApiFinal (
  IN  HASH_HANDLE HashHandle,
  OUT UINT8       *Digest
)
{
  BOOLEAN     Status;
  UINT8       HashPolicy;

  HashPolicy = PcdGet8 (PcdSystemHashPolicy);

  Status = HashFinalInternal (HashPolicy, &HashHandle, &Digest);

  return Status;
}

/**
  The constructor function of BaseHashLib Dxe.

  @param  FileHandle   The handle of FFS header the loaded driver.
  @param  PeiServices  The pointer to the PEI services.

  @retval EFI_SUCCESS           The constructor executes successfully.
  @retval EFI_OUT_OF_RESOURCES  There is no enough resource for the constructor.

**/
EFI_STATUS
EFIAPI
BaseHashLibApiDxeConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEBUG ((DEBUG_INFO,"Calling BaseHashLibApiDxeConstructor.. \n"));

  return EFI_SUCCESS;
}