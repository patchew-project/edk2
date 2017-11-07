/** @Soc.c
  SoC specific Library containg functions to initialize various SoC components

  Copyright 2017 NXP

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Chassis.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/Utils.h>

#include "Soc.h"

VOID
GetSysInfo (
  OUT SYS_INFO *PtrSysInfo
  )
{
  CCSR_GUR *GurBase;
  CCSR_CLOCK *ClkBase;
  UINTN CpuIndex;
  UINT32 TempRcw;
  UINT32 CPllSel;
  UINT32 CplxPll;
  CONST UINT8 CoreCplxPll[8] = {
    [0] = 0,    /* CC1 PPL / 1 */
    [1] = 0,    /* CC1 PPL / 2 */
    [4] = 1,    /* CC2 PPL / 1 */
    [5] = 1,    /* CC2 PPL / 2 */
  };

  CONST UINT8 CoreCplxPllDivisor[8] = {
    [0] = 1,    /* CC1 PPL / 1 */
    [1] = 2,    /* CC1 PPL / 2 */
    [4] = 1,    /* CC2 PPL / 1 */
    [5] = 2,    /* CC2 PPL / 2 */
  };

  UINTN PllCount;
  UINTN FreqCPll[NUM_CC_PLLS];
  UINTN PllRatio[NUM_CC_PLLS];
  UINTN SysClk;

  GurBase = (VOID *)PcdGet64(PcdGutsBaseAddr);
  ClkBase = (VOID *)PcdGet64(PcdClkBaseAddr);
  SysClk = CLK_FREQ;

  InternalMemZeroMem(PtrSysInfo, sizeof (SYS_INFO));

  PtrSysInfo->FreqSystemBus = SysClk;
  PtrSysInfo->FreqDdrBus = SysClk;

  PtrSysInfo->FreqSystemBus *= (GurRead((UINTN)&GurBase->RcwSr[0]) >>
                CHASSIS2_RCWSR0_SYS_PLL_RAT_SHIFT) &
                CHASSIS2_RCWSR0_SYS_PLL_RAT_MASK;
  PtrSysInfo->FreqDdrBus *= (GurRead((UINTN)&GurBase->RcwSr[0]) >>
                CHASSIS2_RCWSR0_MEM_PLL_RAT_SHIFT) &
                CHASSIS2_RCWSR0_MEM_PLL_RAT_MASK;

  for (PllCount = 0; PllCount < NUM_CC_PLLS; PllCount++) {
    PllRatio[PllCount] = (GurRead((UINTN)&ClkBase->PllCgSr[PllCount].PllCnGSr) >> 1) & 0xff;
    if (PllRatio[PllCount] > 4) {
      FreqCPll[PllCount] = SysClk * PllRatio[PllCount];
    } else {
      FreqCPll[PllCount] = PtrSysInfo->FreqSystemBus * PllRatio[PllCount];
    }
  }

  for (CpuIndex = 0; CpuIndex < MAX_CPUS; CpuIndex++) {
    CPllSel = (GurRead((UINTN)&ClkBase->ClkcSr[CpuIndex].ClkCnCSr) >> 27) & 0xf;
    CplxPll = CoreCplxPll[CPllSel];

    PtrSysInfo->FreqProcessor[CpuIndex] = FreqCPll[CplxPll] / CoreCplxPllDivisor[CPllSel];
  }

  TempRcw = GurRead((UINTN)&GurBase->RcwSr[7]);
  switch ((TempRcw & HWA_CGA_M1_CLK_SEL) >> HWA_CGA_M1_CLK_SHIFT) {
  case 2:
    PtrSysInfo->FreqFman[0] = FreqCPll[0] / 2;
    break;
  case 3:
    PtrSysInfo->FreqFman[0] = FreqCPll[0] / 3;
    break;
  case 4:
    PtrSysInfo->FreqFman[0] = FreqCPll[0] / 4;
    break;
  case 5:
    PtrSysInfo->FreqFman[0] = PtrSysInfo->FreqSystemBus;
    break;
  case 6:
    PtrSysInfo->FreqFman[0] = FreqCPll[1] / 2;
    break;
  case 7:
    PtrSysInfo->FreqFman[0] = FreqCPll[1] / 3;
    break;
  default:
    DEBUG((DEBUG_WARN, "Error: Unknown FMan1 clock select!\n"));
    break;
  }
  PtrSysInfo->FreqSdhc = PtrSysInfo->FreqSystemBus/PcdGet32(PcdPlatformFreqDiv);
  PtrSysInfo->FreqQman = PtrSysInfo->FreqSystemBus/PcdGet32(PcdPlatformFreqDiv);
}

/**
  Function to initialize SoC specific constructs
  // CPU Info
  // SoC Personality
  // Board Personality
  // RCW prints
 **/
VOID
SocInit (
  VOID
  )
{
  CHAR8 Buffer[100];
  UINTN CharCount;

  SmmuInit();

  // Initialize the Serial Port
  SerialPortInitialize ();
  CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "\nUEFI firmware (version %s built at %a on %a)\n\r",
    (CHAR16*)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__);
  SerialPortWrite ((UINT8 *) Buffer, CharCount);

  PrintCpuInfo();
  PrintRCW();

  return;
}
