;------------------------------------------------------------------------------ ;
; Copyright (c) 2015 - 2021, Intel Corporation. All rights reserved.<BR>
; SPDX-License-Identifier: BSD-2-Clause-Patent
;
; Module Name:
;
;   MpEqu.inc
;
; Abstract:
;
;   This is the equates file for Multiple Processor support
;
;-------------------------------------------------------------------------------

CPU_SWITCH_STATE_IDLE         equ        0
CPU_SWITCH_STATE_STORED       equ        1
CPU_SWITCH_STATE_LOADED       equ        2

MP_CPU_EXCHANGE_INFO_OFFSET equ (SwitchToRealProcEnd - RendezvousFunnelProcStart)
struc MP_CPU_EXCHANGE_INFO
  .StackStart:           resq 1
  .StackSize:            resq 1
  .CFunction:            resq 1
  .GdtrProfile:          resb 10
  .IdtrProfile:          resb 10
  .BufferStart:          resq 1
  .ModeOffset:           resq 1
  .ApIndex:              resq 1
  .CodeSegment:          resq 1
  .DataSegment:          resq 1
  .EnableExecuteDisable: resq 1
  .Cr3:                  resq 1
  .InitFlag:             resq 1
  .CpuInfo:              resq 1
  .NumApsExecuting:      resq 1
  .CpuMpData:            resq 1
  .InitializeFloatingPointUnits: resq 1
  .ModeTransitionMemory: resd 1
  .ModeTransitionSegment:resw 1
  .ModeHighMemory:       resd 1
  .ModeHighSegment:      resw 1
  .Enable5LevelPaging:   resb 1
  .SevEsIsEnabled:       resb 1
  .GhcbBase:             resq 1
endstruc
