/** @file

  Retrieve TDREPORT_STRUCT structure from TDX

  Copyright (c) 2020 - 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>

#define REPORT_STRUCT_SIZE    1024
#define ADDITIONAL_DATA_SIZE  64

/**
  This function retrieve TDREPORT_STRUCT structure from TDX.
  The struct contains the measurements/configuration information of
  the guest TD that called the function, measurements/configuratio
  information of the TDX-SEAM module and a REPORTMACSTRUCT.
  The REPORTMACSTRUCT is integrity protected with a MAC and
  contains the hash of the measurements and configuration
  as well as additional REPORTDATA provided by the TD software.

  AdditionalData, a 64-byte value, is provided by the guest TD
  to be included in the TDREPORT

  @param[in,out]  Report             Holds the TEREPORT_STRUCT.
  @param[in]      ReportSize         Size of the report. It must be
                                     larger than 1024B.
  @param[in]      AdditionalData     Point to the additional data.
  @param[in]      AdditionalDataSize Size of the additional data.
                                     If AdditionalData != NULL, then
                                     this value must be 64B.

  @return EFI_SUCCESS
  @return EFI_INVALID_PARAMETER
  @return EFI_DEVICE_ERROR

**/
EFI_STATUS
EFIAPI
TdReport(
  IN OUT UINT8  *Report,
  IN  UINT32    ReportSize,
  IN  UINT8     *AdditionalData,
  IN  UINT32    AdditionalDataSize
  )

{
  EFI_STATUS  Status;
  UINT64      *Data;
  UINT64      *Report_Struct;
  UINT64      *Report_Data;
  UINT64      TdCallStatus;

  if(ReportSize < REPORT_STRUCT_SIZE){
    return EFI_INVALID_PARAMETER;
  }

  if(AdditionalData != NULL && AdditionalDataSize != ADDITIONAL_DATA_SIZE){
    return EFI_INVALID_PARAMETER;
  }

  Data = AllocatePages(EFI_SIZE_TO_PAGES(REPORT_STRUCT_SIZE + ADDITIONAL_DATA_SIZE));
  if(Data == NULL){
    return EFI_OUT_OF_RESOURCES;
  }

  Report_Struct = Data;
  Report_Data = Data + REPORT_STRUCT_SIZE;
  if(AdditionalData != NULL){
    CopyMem(Report_Data, AdditionalData, ADDITIONAL_DATA_SIZE);
  }else{
    ZeroMem(Report_Data, ADDITIONAL_DATA_SIZE);
  }

  TdCallStatus = TdCall(TDCALL_TDREPORT, (UINT64)Report_Struct, (UINT64)Report_Data, 0, 0);

  if(TdCallStatus == TDX_EXIT_REASON_SUCCESS){
    Status = EFI_SUCCESS;
  }else if(TdCallStatus == TDX_EXIT_REASON_OPERAND_INVALID){
    Status = EFI_INVALID_PARAMETER;
  }else{
    Status = EFI_DEVICE_ERROR;
  }

  if(Status != EFI_SUCCESS){
    DEBUG((DEBUG_ERROR, "Error returned from TdReport call - 0x%lx\n", TdCallStatus));
  }else{
    CopyMem(Report, Data, REPORT_STRUCT_SIZE);
  }

  FreePages(Data, EFI_SIZE_TO_PAGES(REPORT_STRUCT_SIZE + ADDITIONAL_DATA_SIZE));

  return Status;
}
