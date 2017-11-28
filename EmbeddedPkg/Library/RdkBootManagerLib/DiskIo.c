#include <RdkBootManagerLib.h>

/* See sparse_format.h in AOSP  */
#define SPARSE_HEADER_MAGIC       0xed26ff3a
#define CHUNK_TYPE_RAW            0xCAC1
#define CHUNK_TYPE_FILL           0xCAC2
#define CHUNK_TYPE_DONT_CARE      0xCAC3
#define CHUNK_TYPE_CRC32          0xCAC4

#define PARTITION_NAME_MAX_LENGTH     72/2

#define FLASH_DEVICE_PATH_SIZE(DevPath) ( GetDevicePathSize (DevPath) - \
    sizeof (EFI_DEVICE_PATH_PROTOCOL))

#define IS_ALPHA(Char) (((Char) <= L'z' && (Char) >= L'a') || \
    ((Char) <= L'Z' && (Char) >= L'Z'))

typedef struct _DISKIO_PARTITION_LIST {
  LIST_ENTRY  Link;
  CHAR16      PartitionName[PARTITION_NAME_MAX_LENGTH];
  EFI_HANDLE  PartitionHandle;
} DISKIO_PARTITION_LIST;

typedef struct _SPARSE_HEADER {
  UINT32    Magic;
  UINT16    MajorVersion;
  UINT16    MinorVersion;
  UINT16    FileHeaderSize;
  UINT16    ChunkHeaderSize;
  UINT32    BlockSize;
  UINT32    TotalBlocks;
  UINT32    TotalChunks;
  UINT32    ImageChecksum;
} SPARSE_HEADER;

typedef struct _CHUNK_HEADER {
  UINT16    ChunkType;
  UINT16    Reserved1;
  UINT32    ChunkSize;
  UINT32    TotalSize;
} CHUNK_HEADER;

STATIC LIST_ENTRY       mPartitionListHead;
STATIC EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *mTextOut;
STATIC UINT8        PartitionInited = 0;

/*
 * Helper to free the partition list
 */
STATIC
VOID
FreePartitionList (
    VOID
)
{
  DISKIO_PARTITION_LIST *Entry;
  DISKIO_PARTITION_LIST *NextEntry;

  Entry = (DISKIO_PARTITION_LIST *) GetFirstNode (&mPartitionListHead);
  while (!IsNull (&mPartitionListHead, &Entry->Link)) {
    NextEntry = (DISKIO_PARTITION_LIST *) GetNextNode (&mPartitionListHead, &Entry->Link);

    RemoveEntryList (&Entry->Link);
    FreePool (Entry);

    Entry = NextEntry;
  }
}

/*
 * Read the PartitionName fields from the GPT partition entries, putting them
 * into an allocated array that should later be freed.
 */
STATIC
EFI_STATUS
ReadPartitionEntries (
    IN  EFI_BLOCK_IO_PROTOCOL   *BlockIo,
    OUT EFI_PARTITION_ENTRY   **PartitionEntries
  )
{
  UINTN                       EntrySize;
  UINTN                       NumEntries;
  UINTN                       BufferSize;
  UINT32                      MediaId;
  EFI_PARTITION_TABLE_HEADER  *GptHeader;
  EFI_STATUS                  Status;

  MediaId = BlockIo->Media->MediaId;

  //
  // Read size of Partition entry and number of entries from GPT header
  //
  GptHeader = AllocatePool (BlockIo->Media->BlockSize);
  if (GptHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = BlockIo->ReadBlocks (BlockIo, MediaId, 1, BlockIo->Media->BlockSize, (VOID *) GptHeader);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Check there is a GPT on the media
  if (GptHeader->Header.Signature != EFI_PTAB_HEADER_ID ||
      GptHeader->MyLBA != 1) {
    DEBUG ((DEBUG_ERROR,
        "Fastboot platform: No GPT on flash. "
        "Fastboot on Versatile Express does not support MBR.\n"
    ));
    return EFI_DEVICE_ERROR;
  }

  EntrySize = GptHeader->SizeOfPartitionEntry;
  NumEntries = GptHeader->NumberOfPartitionEntries;

  FreePool (GptHeader);

  ASSERT (EntrySize != 0);
  ASSERT (NumEntries != 0);

  BufferSize = ALIGN_VALUE (EntrySize * NumEntries, BlockIo->Media->BlockSize);
  *PartitionEntries = AllocatePool (BufferSize);
  if (PartitionEntries == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = BlockIo->ReadBlocks (BlockIo, MediaId, 2, BufferSize, (VOID *) *PartitionEntries);
  if (EFI_ERROR (Status)) {
    FreePool (PartitionEntries);
    return Status;
  }

  return Status;
}

/*
 * Initialise: Open the Android NVM device and find the partitions on it. Save them in
 * a list along with the "PartitionName" fields for their GPT entries.
 * We will use these partition names as the key in
 * FastbootPlatformFlashPartition.
 */
EFI_STATUS
InitDiskIo (
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_DEVICE_PATH_PROTOCOL            *FlashDevicePath;
  EFI_DEVICE_PATH_PROTOCOL            *FlashDevicePathDup;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL            *NextNode;
  HARDDRIVE_DEVICE_PATH               *PartitionNode;
  UINTN                               NumHandles;
  EFI_HANDLE                          *AllHandles;
  UINTN                                 LoopIndex;
  EFI_HANDLE                            FlashHandle;
  EFI_BLOCK_IO_PROTOCOL               *FlashBlockIo;
  EFI_PARTITION_ENTRY                 *PartitionEntries;
  DISKIO_PARTITION_LIST               *Entry;

  InitializeListHead (&mPartitionListHead);

  Status = gBS->LocateProtocol (&gEfiSimpleTextOutProtocolGuid, NULL, (VOID **) &mTextOut);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
        "Fastboot platform: Couldn't open Text Output Protocol: %r\n", Status
    ));
    return Status;
  }

  //
  // Get EFI_HANDLES for all the partitions on the block devices pointed to by
  // PcdFastbootFlashDevicePath, also saving their GPT partition labels.
  // There's no way to find all of a device's children, so we get every handle
  // in the system supporting EFI_BLOCK_IO_PROTOCOL and then filter out ones
  // that don't represent partitions on the flash device.
  //

  FlashDevicePath = ConvertTextToDevicePath ((CHAR16*)PcdGetPtr (PcdAndroidFastbootNvmDevicePath));

  //
  // Open the Disk IO protocol on the flash device - this will be used to read
  // partition names out of the GPT entries
  //
  // Create another device path pointer because LocateDevicePath will modify it.
  FlashDevicePathDup = FlashDevicePath;
  Status = gBS->LocateDevicePath (&gEfiBlockIoProtocolGuid, &FlashDevicePathDup, &FlashHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Warning: Couldn't locate Android NVM device (status: %r)\n", Status));
    // Failing to locate partitions should not prevent to do other Android FastBoot actions
    return EFI_SUCCESS;
  }

  Status = gBS->OpenProtocol (
    FlashHandle,
    &gEfiBlockIoProtocolGuid,
    (VOID **) &FlashBlockIo,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Fastboot platform: Couldn't open Android NVM device (status: %r)\n", Status));
    return EFI_DEVICE_ERROR;
  }

  // Read the GPT partition entry array into memory so we can get the partition names
  Status = ReadPartitionEntries (FlashBlockIo, &PartitionEntries);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Warning: Failed to read partitions from Android NVM device (status: %r)\n", Status));
    // Failing to locate partitions should not prevent to do other Android FastBoot actions
    return EFI_SUCCESS;
  }

  // Get every Block IO protocol instance installed in the system
  Status = gBS->LocateHandleBuffer (
    ByProtocol,
    &gEfiBlockIoProtocolGuid,
    NULL,
    &NumHandles,
    &AllHandles
    );
  ASSERT_EFI_ERROR (Status);

  // Filter out handles that aren't children of the flash device
  for (LoopIndex = 0; LoopIndex < NumHandles; LoopIndex++) {
    // Get the device path for the handle
    Status = gBS->OpenProtocol (
      AllHandles[LoopIndex],
      &gEfiDevicePathProtocolGuid,
      (VOID **) &DevicePath,
      gImageHandle,
      NULL,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
      );
    ASSERT_EFI_ERROR (Status);

    // Check if it is a sub-device of the flash device
    if (!CompareMem (DevicePath, FlashDevicePath, FLASH_DEVICE_PATH_SIZE (FlashDevicePath))) {
      // Device path starts with path of flash device. Check it isn't the flash
      // device itself.
      NextNode = NextDevicePathNode (DevicePath);
      if (IsDevicePathEndType (NextNode)) {
        // Create entry
        Entry = AllocatePool (sizeof (DISKIO_PARTITION_LIST));
        if (Entry == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          FreePartitionList ();
          goto Exit;
        }

        // Copy handle and partition name
        Entry->PartitionHandle = AllHandles[LoopIndex];
        StrCpy (Entry->PartitionName, L"ptable");
        InsertTailList (&mPartitionListHead, &Entry->Link);
        continue;
      }

      // Assert that this device path node represents a partition.
      ASSERT (NextNode->Type == MEDIA_DEVICE_PATH &&
          NextNode->SubType == MEDIA_HARDDRIVE_DP);

      PartitionNode = (HARDDRIVE_DEVICE_PATH *) NextNode;

      // Assert that the partition type is GPT. ReadPartitionEntries checks for
      // presence of a GPT, so we should never find MBR partitions.
      // ("MBRType" is a misnomer - this field is actually called "Partition
      //  Format")
      ASSERT (PartitionNode->MBRType == MBR_TYPE_EFI_PARTITION_TABLE_HEADER);

      // The firmware may install a handle for "partition 0", representing the
      // whole device. Ignore it.
      if (PartitionNode->PartitionNumber == 0) {
        continue;
      }

      //
      // Add the partition handle to the list
      //

      // Create entry
      Entry = AllocatePool (sizeof (DISKIO_PARTITION_LIST));
      if (Entry == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
        FreePartitionList ();
        goto Exit;
      }

      // Copy handle and partition name
      Entry->PartitionHandle = AllHandles[LoopIndex];
      StrnCpy (
          Entry->PartitionName,
          PartitionEntries[PartitionNode->PartitionNumber - 1].PartitionName, // Partition numbers start from 1.
          PARTITION_NAME_MAX_LENGTH
      );
      InsertTailList (&mPartitionListHead, &Entry->Link);

      // Print a debug message if the partition label is empty or looks like
      // garbage.
      if (!IS_ALPHA (Entry->PartitionName[0])) {
        DEBUG ((DEBUG_WARN,
            "Warning: Partition %d doesn't seem to have a GPT partition label. "
            "You won't be able to flash it with Fastboot.\n",
            PartitionNode->PartitionNumber
        ));
      }
    }
  }

  Exit:
  FreePool (PartitionEntries);
  FreePool (FlashDevicePath);
  FreePool (AllHandles);
  return Status;

}

STATIC
EFI_STATUS
OpenPartition (
  IN  CHAR8       *PartitionName,
  IN  VOID        *Image,
  IN  UINTN       Size,
  OUT EFI_BLOCK_IO_PROTOCOL     **BlockIo,
  OUT EFI_DISK_IO_PROTOCOL      **DiskIo
  )
{
  EFI_STATUS               Status;
  UINTN                    PartitionSize;
  DISKIO_PARTITION_LIST    *Entry;
  CHAR16                   PartitionNameUnicode[60];
  BOOLEAN                  PartitionFound;
  SPARSE_HEADER            *SparseHeader;

  if(!PartitionInited)
  {
    InitDiskIo();
    PartitionInited = 1;
  }

  AsciiStrToUnicodeStr (PartitionName, PartitionNameUnicode);

  PartitionFound = FALSE;
  Entry = (DISKIO_PARTITION_LIST *) GetFirstNode (&(mPartitionListHead));
  while (!IsNull (&mPartitionListHead, &Entry->Link)) {
    // Search the partition list for the partition named by PartitionName
    if (StrCmp (Entry->PartitionName, PartitionNameUnicode) == 0) {
      PartitionFound = TRUE;
      break;
    }

    Entry = (DISKIO_PARTITION_LIST *) GetNextNode (&mPartitionListHead, &(Entry)->Link);
  }
  if (!PartitionFound) {
    Status = EFI_NOT_FOUND;
    goto exit;
  }

  Status = gBS->OpenProtocol (
    Entry->PartitionHandle,
    &gEfiBlockIoProtocolGuid,
    (VOID **) BlockIo,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to open Block IO protocol: %r\n", Status));
    Status = EFI_NOT_FOUND;
    goto exit;
  }

  SparseHeader=(SPARSE_HEADER *)Image;

  if (SparseHeader->Magic == SPARSE_HEADER_MAGIC) {
    DEBUG ((DEBUG_INFO, "Sparse Magic: 0x%x Major: %d Minor: %d fhs: %d chs: %d bs: %d tbs: %d tcs: %d checksum: %d \n",
      SparseHeader->Magic, SparseHeader->MajorVersion, SparseHeader->MinorVersion,  SparseHeader->FileHeaderSize,
      SparseHeader->ChunkHeaderSize, SparseHeader->BlockSize, SparseHeader->TotalBlocks,
      SparseHeader->TotalChunks, SparseHeader->ImageChecksum));

    if (SparseHeader->MajorVersion != 1) {
      DEBUG ((DEBUG_ERROR, "Sparse image version %d.%d not supported.\n",
            SparseHeader->MajorVersion, SparseHeader->MinorVersion));
      Status = EFI_INVALID_PARAMETER;
      goto exit;
    }

    Size = SparseHeader->BlockSize * SparseHeader->TotalBlocks;
  }

  // Check image will fit on device
  PartitionSize = (BlockIo[0]->Media->LastBlock + 1) * BlockIo[0]->Media->BlockSize;
  if (PartitionSize < Size) {
    DEBUG ((DEBUG_ERROR, "Partition not big enough.\n"));
    DEBUG ((DEBUG_ERROR, "Partition Size:\t%ld\nImage Size:\t%ld\n", PartitionSize, Size));

    Status = EFI_VOLUME_FULL;
    goto exit;
  }

  Status = gBS->OpenProtocol (
    Entry->PartitionHandle,
    &gEfiDiskIoProtocolGuid,
    (VOID **) DiskIo,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

exit:
  return Status;
}

EFI_STATUS
PartitionRead (
  IN CHAR8  *PartitionName,
  IN VOID   *Image,
  IN UINTN  Size
  )
{
  EFI_STATUS               Status;
  EFI_BLOCK_IO_PROTOCOL    *BlockIo;
  EFI_DISK_IO_PROTOCOL     *DiskIo;
  UINT32                   MediaId;

  Status = OpenPartition (PartitionName, Image, Size, &BlockIo, &DiskIo);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  MediaId = BlockIo->Media->MediaId;

  Status = DiskIo->ReadDisk (DiskIo, MediaId, 0, Size, Image);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  BlockIo->FlushBlocks(BlockIo);

exit:
  return Status;
}


EFI_STATUS
PartitionWrite (
  IN CHAR8  *PartitionName,
  IN VOID   *Image,
  IN UINTN  Size
  )
{
  EFI_STATUS               Status;
  EFI_BLOCK_IO_PROTOCOL    *BlockIo;
  EFI_DISK_IO_PROTOCOL     *DiskIo;
  UINT32                   MediaId;
  SPARSE_HEADER            *SparseHeader;
  CHUNK_HEADER             *ChunkHeader;
  UINT32                   Chunk;
  UINTN                    Offset;

  Status = OpenPartition (PartitionName, Image, Size, &BlockIo, &DiskIo);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Offset = 0;
  MediaId = BlockIo->Media->MediaId;
  SparseHeader = (SPARSE_HEADER *)Image;

  if (SparseHeader->Magic == SPARSE_HEADER_MAGIC) {
    CHAR16 OutputString[64];
    UINTN ChunkPrintDensity =
      SparseHeader->TotalChunks > 1600 ? SparseHeader->TotalChunks / 200 : 32;

    Image += SparseHeader->FileHeaderSize;
    for (Chunk = 0; Chunk < SparseHeader->TotalChunks; Chunk++) {
      UINTN WriteSize;
      ChunkHeader = (CHUNK_HEADER *)Image;

      // Show progress. Don't do it for every packet as outputting text
      // might be time consuming. ChunkPrintDensity is calculated to
      // provide an update every half percent change for large
      // downloads.
      if (Chunk % ChunkPrintDensity == 0) {
        UnicodeSPrint(OutputString, sizeof(OutputString),
            L"\r%5d / %5d chunks written (%d%%)", Chunk,
            SparseHeader->TotalChunks,
            (Chunk * 100) / SparseHeader->TotalChunks);
        mTextOut->OutputString(mTextOut, OutputString);
      }

      DEBUG ((DEBUG_INFO, "Chunk #%d - Type: 0x%x Size: %d TotalSize: %d Offset %d\n",
            (Chunk+1), ChunkHeader->ChunkType, ChunkHeader->ChunkSize,
            ChunkHeader->TotalSize, Offset));
      Image += sizeof(CHUNK_HEADER);
      WriteSize=(SparseHeader->BlockSize) * ChunkHeader->ChunkSize;
      switch (ChunkHeader->ChunkType) {
        case CHUNK_TYPE_RAW:
          DEBUG ((DEBUG_INFO, "Writing %d at Offset %d\n", WriteSize, Offset));
          Status = DiskIo->WriteDisk (DiskIo, MediaId, Offset, WriteSize, Image);
          if (EFI_ERROR (Status)) {
            goto exit;
          }
          Image+=WriteSize;
          break;
        case CHUNK_TYPE_DONT_CARE:
          break;
        case CHUNK_TYPE_CRC32:
          break;
        default:
          DEBUG ((DEBUG_ERROR, "Unknown Chunk Type: 0x%x", ChunkHeader->ChunkType));
          Status = EFI_PROTOCOL_ERROR;
          goto exit;
      }
      Offset += WriteSize;
    }

    UnicodeSPrint(OutputString, sizeof(OutputString),
        L"\r%5d / %5d chunks written (100%%)\r\n",
        SparseHeader->TotalChunks, SparseHeader->TotalChunks);
    mTextOut->OutputString(mTextOut, OutputString);

  } else {

    Status = DiskIo->WriteDisk (DiskIo, MediaId, 0, Size, Image);
    if (EFI_ERROR (Status)) {
      goto exit;
    }
  }

  BlockIo->FlushBlocks(BlockIo);

exit:
  return Status;
}
