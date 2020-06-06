//
//  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
//  Copyright (c) 2015-2020, Linaro Limited. All rights reserved.
//
//  SPDX-License-Identifier: BSD-2-Clause-Patent
//
//

#include <AsmMacroIoLibV8.h>
ASM_FUNC(_ModuleEntryPoint)
  adr   x8, FeaturePcdGet (PcdStMMReloc)
  ldrb  w9, [x8]
  cmp   x9, #0
  // If PcdStMMReloc is set to TRUE, go ahead and fixup the relocations here
  beq   .Llaunch
  adr   x8, __reloc_base
  adr   x9, __reloc_start
  adr   x10, __reloc_end
  bl    ASM_PFX(_DoReloc)
.Llaunch:
  b     ModuleEntryPoint

.section .text.reloc, "ax"
ASM_PFX(_DoReloc):
  mov x20, x0
  mov x21, x1
  mov x22, x2
  mov x23, x3

  // Set all of the memory as r/w
  adr x11, __stmm_start
  adr x2, __stmm_end
  ldr x0, =0xC4000065
  and x1, x11, #~0x0fff      // Align to page
  sub x2, x2, x11
  add x2, x2, #0xfff
  lsr x2, x2, #12            // nr pages
  ldr x3, =0x5               // Set perms to r/w
  svc #0
  // Prevent speculative execution beyond svc instruction
  dsb nsh
  isb

.Lreloc_loop:
  cmp   x9, x10
  bhs   .Lreloc_done
  //
  // We are built as a ET_DYN PIE executable, so we need to process all
  // relative relocations regardless of whether or not we are executing from
  // the same offset we were linked at. This is only possible if we are
  // running from RAM.
  //
  // AArch64 uses the ELF64 RELA format, which means each entry in the
  // relocation table consists of
  //
  //   UINT64 offset          : the relative offset of the value that needs to
  //                            be relocated
  //   UINT64 info            : relocation type and symbol index (the latter is
  //                            not used for R_AARCH64_RELATIVE relocations)
  //   UINT64 addend          : value to be added to the value being relocated
  //
  ldp   x11, x12, [x9], #24   // read offset into x11 and info into x12
  cmp   x12, #0x403           // check info == R_AARCH64_RELATIVE?
  bne   .Lreloc_loop          // not a relative relocation? then skip
  ldr   x12, [x9, #-8]        // read addend into x12
  add   x12, x12, x8          // add reloc base to addend to get relocated value
  str   x12, [x11, x8]        // write relocated value at offset
  b     .Lreloc_loop

.Lreloc_done:
  // set memory per section
  adr x11, __stmm_start
  adr x2, __reloc_end
  ldr x0, =0xC4000065
  and x1, x11, #~0x0fff      // Align to page
  sub x2, x2, x11
  add x2, x2, #0xfff
  lsr x2, x2, #12            // nr pages
  ldr x3, =0x3               // Set perms to r/x
  svc #0
  // Prevent speculative execution beyond svc instruction
  dsb nsh
  isb

  adr   x11, __ro_start
  adr   x2, __ro_end
  ldr x0, =0xC4000065
  and x1, x11, #~0x0fff      // Align to page
  sub x2, x2, x11
  add x2, x2, #0xfff
  lsr x2, x2, #12            // nr pages
  ldr x3, =0x1               // Set perms to r/o
  svc #0
  // Prevent speculative execution beyond svc instruction
  dsb nsh
  isb

  mov x0, x20
  mov x1, x21
  mov x2, x22
  mov x3, x23

  ret
