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
  StrCat(*Dest, String1);
  StrCat(*Dest, String2);
}

STATIC
EFI_STATUS
LsFiles (
  IN  CONST CHAR16  *DirPath,
  IN  CONST CHAR16  *TargetFile,
  OUT CHAR16        **Result,
  IN  LIST_HEAD     *Head
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
      ListAdd(&Node->List, Head);
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
  IN  LIST_HEAD *Head
  )
{
  DIR_NODE  *Node;
  DIR_NODE  *Temp;

  LIST_FOR_EACH_ENTRY_SAFE (Node, Temp, Head, List) {
    ListDel(&Node->List);
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
  LIST_HEAD   DirList[2];

  *Result           = NULL;
  EFI_STATUS Status = EFI_NOT_FOUND;

  INIT_LIST_HEAD(&DirList[0]);
  INIT_LIST_HEAD(&DirList[1]);

  for (Current = 0, LsFiles(DevPath, TargetFile, Result, &DirList[Current]);
      !ListEmpty(&DirList[Current]);
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

  for(OuterLoopIndex=0, Next=0; OuterLoopIndex<MAX_VAR &&  Next < FileSize; OuterLoopIndex++) {
    for(InnerLoopIndex=0; InnerLoopIndex<2; InnerLoopIndex++) {
      Current = Next;
      Next += StrSpn(&FileData[Next], VarDelimiter[InnerLoopIndex]);
      FileData[Next] = '\0';
      VarResult[OuterLoopIndex][InnerLoopIndex] = Ascii2Uefi(&FileData[Current]);
      Next += 2;
    }
  }

  if(OuterLoopIndex < MAX_VAR) {
    Status = EFI_NOT_FOUND;
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
  CHAR16        **Result;
  CONST CHAR16  *DevPath;
  CONST CHAR16  *RdkFileName;

  Result      = NULL;
  DevPath     = (CONST CHAR16 *)FixedPcdGetPtr (PcdRdkConfFileDevicePath);
  RdkFileName = (CONST CHAR16 *)FixedPcdGetPtr (PcdRdkConfFileName);

  Status = FindFileInDir(DevPath, RdkFileName, Result);
  if(EFI_ERROR(Status)) {
    return Status;
  }

  Status = RdkReadFile ((CONST CHAR16 *)*Result, (VOID**) &RdkData, &RdkSize);
  if(EFI_ERROR(Status)) {
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

  for(Count=0; Count<MAX_VAR; Count++) {
    if(StrCmp(Name, VarResult[Count][0]) == 0) {
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
