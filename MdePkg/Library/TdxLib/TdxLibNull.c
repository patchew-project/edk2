/** @file
  Null instance of TdxLib.

  Copyright (c) 2020 - 2021, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>

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
  IN UINT32     ReportSize,
  IN UINT8      *AdditionalData,
  IN UINT32     AdditionalDataSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  This function accept a pending private page, and initialize the page to
  all-0 using the TD ephemeral private key.

  @param[in]  StartAddress     Guest physical address of the private page
                               to accept.
  @param[in]  NumberOfPages    Number of the pages to be accepted.

  @return EFI_UNSUPPORTED
**/
EFI_STATUS
EFIAPI
TdAcceptPages (
  IN UINT64  StartAddress,
  IN UINT64  NumberOfPages
  )
{
  return EFI_UNSUPPORTED;
}

/**
  The TDCALL instruction causes a VM exit to the Intel TDX module.  It is
  used to call guest-side Intel TDX functions, either local or a TD exit
  to the host VMM, as selected by Leaf.
  Leaf functions are described at <https://software.intel.com/content/
  www/us/en/develop/articles/intel-trust-domain-extensions.html>

  @param[in]      Leaf        Leaf number of TDCALL instruction
  @param[in]      Arg1        Arg1
  @param[in]      Arg2        Arg2
  @param[in]      Arg3        Arg3
  @param[in,out]  Results  Returned result of the Leaf function

  @return EFI_SUCCESS
  @return Other           See individual leaf functions
**/
EFI_STATUS
EFIAPI
TdCall(
  IN UINT64           Leaf,
  IN UINT64           Arg1,
  IN UINT64           Arg2,
  IN UINT64           Arg3,
  IN OUT VOID         *Results
  )
{
  return EFI_UNSUPPORTED;
}

/**
  TDVMALL is a leaf function 0 for TDCALL. It helps invoke services from the
  host VMM to pass/receive information.

  @param[in]     Leaf        Number of sub-functions
  @param[in]     Arg1        Arg1
  @param[in]     Arg2        Arg2
  @param[in]     Arg3        Arg3
  @param[in]     Arg4        Arg4
  @param[in,out] Results     Returned result of the sub-function

  @return EFI_SUCCESS
  @return Other           See individual sub-functions

**/
EFI_STATUS
EFIAPI
TdVmCall (
  IN UINT64          Leaf,
  IN UINT64          Arg1,
  IN UINT64          Arg2,
  IN UINT64          Arg3,
  IN UINT64          Arg4,
  IN OUT VOID        *Results
  )
{
  return EFI_UNSUPPORTED;
}

/**
  This function enable the TD guest to request the VMM to emulate CPUID
  operation, especially for non-architectural, CPUID leaves.

  @param[in]     Eax        Main leaf of the CPUID
  @param[in]     Ecx        Sub-leaf of the CPUID
  @param[in,out] Results    Returned result of CPUID operation

  @return EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
TdVmCallCpuid (
  IN UINT64         Eax,
  IN UINT64         Ecx,
  IN OUT VOID       *Results
  )
{
  return EFI_UNSUPPORTED;
}
