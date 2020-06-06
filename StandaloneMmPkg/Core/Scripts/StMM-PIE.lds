/** @file

  Copyright (c) 2015, Linaro Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

SECTIONS
{
  PROVIDE(__reloc_base = .);

  . = PECOFF_HEADER_SIZE;
  .text : ALIGN(CONSTANT(COMMONPAGESIZE)) {
    *(.text.reloc*)
    . = ALIGN(0x1000);
    PROVIDE(__stmm_start = .);
    *(.text .text*)
    . = ALIGN(0x20);
    PROVIDE(__reloc_start = .);
    *(.rel .rel.*)
    *(.rela .rela.*)
    PROVIDE(__reloc_end = .);
  }
  PROVIDE(__ro_start = .);
  .got : ALIGN(CONSTANT(COMMONPAGESIZE)) {
    *(.got .got*)
  }
  .rodata : ALIGN(CONSTANT(COMMONPAGESIZE)) {
    *(.rodata .rodata*)
  }
  PROVIDE(__ro_end = .);
  .data : ALIGN(CONSTANT(COMMONPAGESIZE)) {
    *(.data .data*)
  }
  .bss : ALIGN(CONSTANT(COMMONPAGESIZE)) {
    *(.bss .bss*)
  }
  PROVIDE(__stmm_end = .);

  .note (INFO) : { *(.note.gnu.build-id) }

  /DISCARD/ : {
    *(.note.GNU-stack)
    *(.gnu.hash)
    *(.gnu_debuglink)
    *(.interp)
    *(.dynamic)
    *(.dynsym)
    *(.dynstr)
    *(.hash)
    *(.comment)
  }
}
