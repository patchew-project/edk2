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
  .StackStart:           resd 1
  .StackSize:            resd 1
  .CFunction:            resd 1
  .GdtrProfile:          resb 6
  .IdtrProfile:          resb 6
  .BufferStart:          resd 1
  .ModeOffset:           resd 1
  .ApIndex:              resd 1
  .CodeSegment:          resd 1
  .DataSegment:          resd 1
  .EnableExecuteDisable: resd 1
  .Cr3:                  resd 1
  .InitFlag:             resd 1
  .CpuInfo:              resd 1
  .NumApsExecuting:      resd 1
  .CpuMpData:            resd 1
  .InitializeFloatingPointUnits: resd 1
  .ModeTransitionMemory: resd 1
  .ModeTransitionSegment:resw 1
  .ModeHighMemory:       resd 1
  .ModeHighSegment:      resw 1
  .Enable5LevelPaging:   resb 1
  .SevEsIsEnabled:       resb 1
  .GhcbBase:             resd 1
endstruc
