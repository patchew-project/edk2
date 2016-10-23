/** @file
  Parse the INI configuration file and pass the information to the recovery driver
  so that the driver can perform recovery accordingly.

  The Config file format is below:

  [Head]
  NumOfRecovery = <Num>     # Decimal
  Recovery0 = <Name1>       # String
  Recovery1 = <Name2>       # String
  Recovery<Num-1> = <NameX> # String

  [Name?]
  Length      = <Length>         # Fv Length (HEX)
  ImageOffset = <ImageOffset>    # Fv offset of this SystemFirmware image (HEX)
  FileGuid    = XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX  # PcdEdkiiSystemFirmwareFileGuid

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions
  of the BSD License which accompanies this distribution.  The
  full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "RecoveryModuleLoadPei.h"
#include <Library/IniParsingLib.h>
#include <Library/PrintLib.h>

#define MAX_LINE_LENGTH           512

/**
  Parse Config data file to get the updated data array.

  @param DataBuffer      Config raw file buffer.
  @param BufferSize      Size of raw buffer.
  @param ConfigHeader    Pointer to the config header.
  @param RecoveryArray   Pointer to the config of recovery data.

  @retval EFI_NOT_FOUND         No config data is found.
  @retval EFI_OUT_OF_RESOURCES  No enough memory is allocated.
  @retval EFI_SUCCESS           Parse the config file successfully.

**/
EFI_STATUS
ParseRecoveryDataFile (
  IN      UINT8                         *DataBuffer,
  IN      UINTN                         BufferSize,
  IN OUT  CONFIG_HEADER                 *ConfigHeader,
  IN OUT  RECOVERY_CONFIG_DATA          **RecoveryArray
  )
{
  EFI_STATUS                            Status;
  CHAR8                                 *SectionName;
  CHAR8                                 Entry[MAX_LINE_LENGTH];
  UINTN                                 Num;
  UINTN                                 Index;
  EFI_GUID                              FileGuid;
  VOID                                  *Context;

  //
  // First process the data buffer and get all sections and entries
  //
  Context = OpenIniFile(DataBuffer, BufferSize);
  if (Context == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Now get NumOfUpdate
  //
  Status = GetDecimalUintnFromDataFile(
             Context,
             "Head",
             "NumOfRecovery",
             &Num
             );
  if (EFI_ERROR(Status) || (Num == 0)) {
    DEBUG((EFI_D_ERROR, "NumOfRecovery not found\n"));
    CloseIniFile(Context);
    return EFI_NOT_FOUND;
  }

  ConfigHeader->NumOfRecovery = Num;
  *RecoveryArray = AllocateZeroPool ((sizeof (RECOVERY_CONFIG_DATA) * Num));
  if (*RecoveryArray == NULL) {
    CloseIniFile(Context);
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0 ; Index < ConfigHeader->NumOfRecovery; Index++) {
    //
    // Get the section name of each update
    //
    AsciiStrCpyS (Entry, MAX_LINE_LENGTH, "Recovery");
    AsciiValueToString(Entry + AsciiStrLen(Entry), 0, Index, 0);
    Status = GetStringFromDataFile(
               Context,
               "Head",
               Entry,
               &SectionName
               );
    if (EFI_ERROR(Status) || (SectionName == NULL)) {
      DEBUG((EFI_D_ERROR, "[%d] %a not found\n", Index, Entry));
      CloseIniFile(Context);
      return EFI_NOT_FOUND;
    }

    //
    // The section name of this update has been found.
    // Now looks for all the config data of this update
    //

    //
    // FileBuid
    //
    Status = GetGuidFromDataFile(
               Context,
               SectionName,
               "FileGuid",
               &FileGuid
               );
    if (EFI_ERROR(Status)) {
      CloseIniFile(Context);
      DEBUG((EFI_D_ERROR, "[%d] FileGuid not found\n", Index));
      return EFI_NOT_FOUND;
    }

    CopyGuid(&((*RecoveryArray)[Index].FileGuid), &FileGuid);

    //
    // Length
    //
    Status = GetHexUintnFromDataFile(
               Context,
               SectionName,
               "Length",
               &Num
               );
    if (EFI_ERROR(Status)) {
      CloseIniFile(Context);
      DEBUG((EFI_D_ERROR, "[%d] Length not found\n", Index));
      return EFI_NOT_FOUND;
    }
    (*RecoveryArray)[Index].Length = Num;

    //
    // ImageOffset
    //
    Status = GetHexUintnFromDataFile(
               Context,
               SectionName,
               "ImageOffset",
               &Num
               );
    if (EFI_ERROR(Status)) {
      CloseIniFile(Context);
      DEBUG((EFI_D_ERROR, "[%d] ImageOffset not found\n", Index));
      return EFI_NOT_FOUND;
    }
    (*RecoveryArray)[Index].ImageOffset = Num;
  }

  //
  // Now all configuration data got. Free those temporary buffers
  //
  CloseIniFile(Context);

  return EFI_SUCCESS;
}

