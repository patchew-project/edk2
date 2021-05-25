/** @file
  SEV Hash finder library to locate the SEV encrypted hash table

  Copyright (C) 2021 James Bottomley, IBM Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/DebugLib.h>
#include <Library/SevHashFinderLib.h>

#pragma pack (1)
typedef struct {
  GUID   Guid;
  UINT16 Len;
  UINT8  Data[];
} HASH_TABLE;
#pragma pack ()

STATIC HASH_TABLE *mHashTable;
STATIC UINT16 mHashTableSize;

EFI_STATUS
EFIAPI
ValidateHashEntry (
  IN CONST GUID *Guid,
  IN CONST VOID *Buf,
  UINT32 BufSize
  )
{
  INT32 Len;
  HASH_TABLE *Entry;
  UINT8 Hash[SHA256_DIGEST_SIZE];

  if (mHashTable == NULL || mHashTableSize == 0) {
    DEBUG ((DEBUG_ERROR,
      "%a: Verifier Called but no hash table discoverd in MEMFD\n",
      __FUNCTION__));
    return EFI_ACCESS_DENIED;
  }

  Sha256HashAll (Buf, BufSize, Hash);

  for (Entry = mHashTable, Len = 0;
       Len < (INT32)mHashTableSize;
       Len += Entry->Len,
       Entry = (HASH_TABLE *)((UINT8 *)Entry + Entry->Len)) {
    UINTN EntrySize;
    EFI_STATUS Status;

    if (!CompareGuid (&Entry->Guid, Guid)) {
      continue;
    }

    DEBUG ((DEBUG_INFO, "%a: Found GUID %g in table\n", __FUNCTION__, Guid));

    //
    // Verify that the buffer's hash is identical to the hash table entry
    //
    EntrySize = Entry->Len - sizeof (Entry->Guid) - sizeof (Entry->Len);
    if (EntrySize != SHA256_DIGEST_SIZE) {
      DEBUG ((DEBUG_ERROR, "%a: Hash has the wrong size %d != %d\n",
        __FUNCTION__, EntrySize, SHA256_DIGEST_SIZE));
      return EFI_ACCESS_DENIED;
    }
    if (CompareMem (Entry->Data, Hash, EntrySize) == 0) {
      Status = EFI_SUCCESS;
      DEBUG ((DEBUG_INFO, "%a: Hash Comparison succeeded\n", __FUNCTION__));
    } else {
      Status = EFI_ACCESS_DENIED;
      DEBUG ((DEBUG_ERROR, "%a: Hash Comparison Failed\n", __FUNCTION__));
    }
    return Status;
  }
  DEBUG ((DEBUG_ERROR, "%a: Hash GUID %g not found in table\n", __FUNCTION__,
    Guid));
  return EFI_ACCESS_DENIED;
}

/**
  Register security measurement handler.

  This function always returns success, even if the table
  can't be found.  It only returns errors if an actual use
  is made of the non-existent table because that indicates it
  should have been present.

  @param  ImageHandle   ImageHandle of the loaded driver.
  @param  SystemTable   Pointer to the EFI System Table.

  @retval EFI_SUCCESS   The verifier tables were set up correctly
**/
EFI_STATUS
EFIAPI
SevHashFinderLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  HASH_TABLE *Ptr = (void *)FixedPcdGet64 (PcdQemuHashTableBase);
  UINT32 Size = FixedPcdGet32 (PcdQemuHashTableSize);

  mHashTable = NULL;
  mHashTableSize = 0;

  if (Ptr == NULL || Size == 0) {
    return EFI_SUCCESS;
  }

  if (!CompareGuid (&Ptr->Guid, &SEV_HASH_TABLE_GUID)) {
    return EFI_SUCCESS;
  }

  DEBUG ((DEBUG_INFO, "%a: found Injected Hash in secure location\n",
    __FUNCTION__));

  mHashTable = (HASH_TABLE *)Ptr->Data;
  mHashTableSize = Ptr->Len - sizeof (Ptr->Guid) - sizeof (Ptr->Len);

  DEBUG ((DEBUG_INFO, "%a: Ptr=%p, Size=%d\n", __FUNCTION__, mHashTable,
    mHashTableSize));

  return EFI_SUCCESS;
}
