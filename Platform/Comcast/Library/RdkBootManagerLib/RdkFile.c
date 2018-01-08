#include <RdkBootManagerLib.h>

STATIC UINT8    VarablesInitialzed = 0;
STATIC CHAR16   *VarResult[MAX_VAR][2];

STATIC
VOID
SaveString (
  OUT CHAR16    **Dest,
  IN  CHAR16    *String1,
  IN  CHAR16    *String2
  )
{
  *Dest = ALLOCATE_STRING_MEM(StrLen(String1) + StrLen(String2));
  ASSERT( NULL != Dest );
  StrCat(*Dest, String1);
  StrCat(*Dest, String2);
}

STATIC
EFI_STATUS
LsFiles (
  IN  CONST CHAR16  *DirPath,
  IN  CONST CHAR16  *TargetFile,
  OUT CHAR16        **Result,
  IN  LIST_ENTRY    *Head
  )
{
  EFI_STATUS          Status;
  EFI_FILE_INFO       *FileInfo;
  EFI_FILE_PROTOCOL   *FileHandle;
  BOOLEAN             NoFile;
  CHAR16              *TempPath;
  DIR_NODE            *Node;

  NoFile    = FALSE;
  TempPath  = ALLOCATE_STRING_MEM(StrLen(DirPath) + 1);
  StrCat(TempPath, DirPath);
  StrCat(TempPath, L"/");

  Status = GetFileHandler(&FileHandle, DirPath, EFI_FILE_MODE_READ);
  ASSERT_EFI_ERROR(Status);

  for ( Status = FileHandleFindFirstFile(FileHandle, &FileInfo)
      ; !EFI_ERROR(Status) && !NoFile
      ; Status = FileHandleFindNextFile(FileHandle, FileInfo, &NoFile)
      ) {
    if((FileInfo->Attribute & EFI_FILE_DIRECTORY) &&
        (StrCmp(FileInfo->FileName, L".") != 0) &&
        (StrCmp(FileInfo->FileName, L"..") != 0)) {
      Node = AllocateZeroPool(sizeof (DIR_NODE));
      SaveString(&Node->Name, TempPath, FileInfo->FileName);
      InsertHeadList(Head,&Node->List);
    }
    else if(StrCmp(FileInfo->FileName, TargetFile) == 0) {
      SaveString(Result, TempPath, FileInfo->FileName);
      Status = EFI_SUCCESS;
      goto ON_EXIT;
    }
  }

  Status = EFI_NOT_FOUND;

ON_EXIT:
  FreePool(TempPath);
  return Status;
}

STATIC
VOID
DelDirList (
  IN  LIST_ENTRY *Head
  )
{
  DIR_NODE  *Node;
  DIR_NODE  *Temp;

  LIST_FOR_EACH_ENTRY_SAFE (Node, Temp, Head, List) {
    RemoveEntryList(&Node->List);
    FreePool(Node->Name);
    FreePool(Node);
  }
}

STATIC
EFI_STATUS
FindFileInDir (
  IN  CONST CHAR16  *DevPath,
  IN  CONST CHAR16  *TargetFile,
  OUT CHAR16    **Result
  )
{
  UINT8       Current;
  UINT8       Next;
  DIR_NODE    *Temp;
  LIST_ENTRY  DirList[2];

  *Result           = NULL;
  EFI_STATUS Status = EFI_NOT_FOUND;

  InitializeListHead(&DirList[0]);
  InitializeListHead(&DirList[1]);

  for (Current = Next = 0, Status=LsFiles(DevPath, TargetFile, Result, &DirList[Current]);
      !IsListEmpty(&DirList[Current]);
      Current = Next) {
    Next = Current ^ 1;
    DelDirList(&DirList[Next]);

    LIST_FOR_EACH_ENTRY(Temp, &DirList[Current], List) {
      Status = LsFiles(Temp->Name, TargetFile, Result, &DirList[Next]);
      if(!EFI_ERROR(Status)) {
        DelDirList(&DirList[Current]);
        break;
      }
    }
  }

  DelDirList(&DirList[Next]);
  return Status;
}

STATIC
UINTN
StrSpn (
  IN CHAR8    *String,
  IN CHAR8    *CharSet
  )
{
  UINTN Count;

  for(Count=0; String[Count] && !(String[Count] == CharSet[0]); Count++);
  return Count;
}

STATIC
CHAR16 *
Ascii2Uefi (
  IN CHAR8  *String
  )
{
  CHAR16  *Result;
  UINTN   Size;

  Size    = AsciiStrLen(String);
  Result  = ALLOCATE_STRING_MEM(Size);

  while(Size--) {
    Result[Size] = String[Size];
  }

  return Result;
}

STATIC
EFI_STATUS
InitVarList (
  IN  CHAR8  *FileData,
  IN  UINTN   FileSize
  )
{
  UINTN       InnerLoopIndex;
  UINTN       OuterLoopIndex;
  UINTN       Current;
  UINTN       Next;
  CHAR8       *VarDelimiter[2];
  EFI_STATUS  Status;

  VarDelimiter[0] = "=";
  VarDelimiter[1] = "\"";
  Status          = EFI_SUCCESS;

  //Initialize to NULL
  for(OuterLoopIndex=0; OuterLoopIndex < MAX_VAR; OuterLoopIndex++) {
      VarResult[OuterLoopIndex][0] = VarResult[OuterLoopIndex][1] = NULL;
  }

  for(OuterLoopIndex=0, Next=0; OuterLoopIndex < MAX_VAR && Next < FileSize; OuterLoopIndex++) {
    for(InnerLoopIndex=0; InnerLoopIndex < 2; InnerLoopIndex++) {
      Current = Next;
      Next += StrSpn(&FileData[Next], VarDelimiter[InnerLoopIndex]);
      FileData[Next] = '\0';
      VarResult[OuterLoopIndex][InnerLoopIndex] = Ascii2Uefi(&FileData[Current]);
      //skip new line
      Next += 2;
    }
  }

  return Status;
}

STATIC
EFI_STATUS
InitRdkVariables (
  VOID
  )
{
  EFI_STATUS    Status;
  UINTN         RdkSize;
  UINT8         *RdkData;
  CHAR16        *Result;
  CONST CHAR16  *DevPath;
  CONST CHAR16  *RdkFileName;

  DevPath     = (CONST CHAR16 *)FixedPcdGetPtr (PcdRdkConfFileDevicePath);
  RdkFileName = (CONST CHAR16 *)FixedPcdGetPtr (PcdRdkConfFileName);

  Status = FindFileInDir(DevPath, RdkFileName, &Result);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to find file %s in %s\n", RdkFileName, DevPath));
    return Status;
  }

  Status = RdkReadFile ((CONST CHAR16 *)Result, (VOID**) &RdkData, &RdkSize);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to read file %s\n", RdkFileName));
    return Status;
  }

  Status = InitVarList ((CHAR8 *)RdkData, RdkSize);
  return Status;
}

STATIC
EFI_STATUS
GetVarValue (
  IN  CONST CHAR16 *Name,
  OUT CONST CHAR16 **Value
  )
{
  UINTN         Count;
  EFI_STATUS    Status;

  if(!VarablesInitialzed) {
    Status = InitRdkVariables();
    if(EFI_ERROR(Status)) {
      return Status;
    }

    VarablesInitialzed = 1;
  }

  //Initialize to NULL
  *Value = NULL;

  for(Count=0; Count<MAX_VAR; Count++) {
    if(NULL != VarResult[Count][0] && StrCmp(Name, VarResult[Count][0]) == 0) {
      *Value = VarResult[Count][1];
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
GetRdkVariable (
  IN  CONST CHAR16  *Name,
  OUT CONST CHAR16  **Value
  )
{
  EFI_STATUS  Status;

  Status = GetVarValue(Name, Value);
  return Status;
}

EFI_STATUS
RdkReadFile (
    IN      CONST CHAR16  *Path,
    IN OUT  VOID          **BufferPtr,
    OUT     UINTN         *FileSize
)
{
    UINTN             BufferSize;
    UINT64            SourceFileSize;
    VOID              *Buffer;
    EFI_STATUS        Status;
    EFI_FILE_HANDLE   FileHandle;

    Status = GetFileHandler(&FileHandle, Path, EFI_FILE_MODE_READ);
    ASSERT_EFI_ERROR(Status);

    Buffer = NULL;

    // Get the file size
    Status = FileHandle->SetPosition (FileHandle, (UINT64) -1);
    if (EFI_ERROR (Status)) {
        goto ON_EXIT;
    }

    Status = FileHandle->GetPosition (FileHandle, &SourceFileSize);
    if (EFI_ERROR (Status)) {
        goto ON_EXIT;
    }

    Status = FileHandle->SetPosition (FileHandle, 0);
    if (EFI_ERROR (Status)) {
        goto ON_EXIT;
    }

    BufferSize = (UINTN) SourceFileSize;
    Buffer =  AllocateZeroPool(BufferSize);
    if (Buffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    if (FileSize != NULL) *FileSize  = BufferSize;

    Status = FileHandle->Read (FileHandle, &BufferSize, Buffer);
    if (EFI_ERROR (Status) || BufferSize != SourceFileSize) {
        FreePool (Buffer);
        Buffer = NULL;
        Status  = EFI_BAD_BUFFER_SIZE;
        goto ON_EXIT;
    }

ON_EXIT:

    *BufferPtr = Buffer;
    return Status;
}

EFI_STATUS
RdkWriteFile (
    IN      CONST CHAR16    *Path,
    IN OUT  VOID            **BufferPtr,
    OUT     UINTN           *FileSize
)
{
    EFI_STATUS        Status;
    EFI_FILE_HANDLE   FileHandle;

    if (FileSize == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    Status = GetFileHandler(&FileHandle, Path, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE);
    ASSERT_EFI_ERROR(Status);

    Status = FileHandle->Write (FileHandle, FileSize, *BufferPtr);
    ASSERT_EFI_ERROR (Status);

    return Status;
}
