/*
** ###################################################################
**     Processor:           MIMXRT1187CVM8B_cm7
**     Compiler:            GNU C Compiler
**     Reference manual:    IMXRT1180RM, Rev 1, 06/2022
**     Version:             rev. 0.1, 2021-03-09
**     Build:               b220810
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2022 NXP
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/

/* Entry Point */
ENTRY(Reset_Handler)

vector_table_size  = 0x00000400;
m_itcm_start       = 0x00000000;
m_itcm_size        = 0x00040000;
m_dtcm_start       = 0x20000000;
m_dtcm_size        = 0x00040000;
m_ocram1_start     = 0x20480000;
m_ocram1_size      = 0x00080000;
m_ocram2_start     = 0x20500000;
m_ocram2_size      = 0x00020000;
m_sdram_start      = 0x80000000;
m_sdram_size       = 0x02000000;
m_sdram_data_size  = 0x01000000;
m_flash_start      = 0x28000000;
m_flash_size       = 0x01000000;
app_image_offset   = 0x0000B000;

VECTOR_RAM_SIZE    = DEFINED(__ram_vector_table__) ? vector_table_size : 0;
HEAP_SIZE          = DEFINED(__heap_size__)  ? __heap_size__  : 0x0400;
STACK_SIZE         = DEFINED(__stack_size__) ? __stack_size__ : 0x0400;
RPMSG_SHMEM_SIZE   = DEFINED(__use_shmem__) ? 0x8000 : 0;
NCACHE_HEAP_START  = DEFINED(__heap_noncacheable__) ? 0x20520000 - HEAP_SIZE : m_ocram1_start + m_ocram1_size - HEAP_SIZE;
NCACHE_HEAP_SIZE   = DEFINED(__heap_noncacheable__) ? HEAP_SIZE : 0x0000;

/* Specify the memory areas */
MEMORY
{
  m_interrupts_ram (RX)  : ORIGIN = m_itcm_start, LENGTH = VECTOR_RAM_SIZE
  m_interrupts     (RX)  : ORIGIN = m_itcm_start, LENGTH = vector_table_size
  m_text           (RX)  : ORIGIN = m_itcm_start + vector_table_size, LENGTH = m_itcm_size - vector_table_size
  m_data           (RW)  : ORIGIN = m_dtcm_start, LENGTH = DEFINED(__heap_noncacheable__) ? m_dtcm_size : m_dtcm_size - HEAP_SIZE
  m_data2          (RW)  : ORIGIN = m_ocram1_start, LENGTH = m_ocram1_size
  m_ncache         (RW)  : ORIGIN = m_ocram1_start, LENGTH = DEFINED(__heap_noncacheable__) ? m_ocram1_size - HEAP_SIZE : m_ocram1_size
  m_heap           (RW)  : ORIGIN = NCACHE_HEAP_START, LENGTH = HEAP_SIZE
  rpmsg_sh_mem     (RW)  : ORIGIN = 0x20520000, LENGTH = RPMSG_SHMEM_SIZE
}

/* Define output sections */
SECTIONS
{
  __NCACHE_REGION_START = ORIGIN(m_ncache);
  __NCACHE_REGION_SIZE  = LENGTH(m_ncache) + NCACHE_HEAP_SIZE;

  __RPMSG_SH_MEM_START = ORIGIN(rpmsg_sh_mem);
  __RPMSG_SH_MEM_SIZE  = LENGTH(rpmsg_sh_mem);

  /* NOINIT section for rpmsg_sh_mem */
  .noinit_rpmsg_sh_mem (NOLOAD) : ALIGN(4)
  {
     __RPMSG_SH_MEM_START__ = .;
     *(.noinit.$rpmsg_sh_mem*)
     . = ALIGN(4) ;
     __RPMSG_SH_MEM_END__ = .;
  } > rpmsg_sh_mem

  /* The startup code goes first into internal RAM */
  .interrupts :
  {
    __VECTOR_TABLE = .;
    __Vectors = .;
    . = ALIGN(4);
    KEEP(*(.isr_vector))     /* Startup code */
    . = ALIGN(4);
  } > m_interrupts

  /* The program code and other data goes into internal RAM */
  .text :
  {
    . = ALIGN(4);
    *(.text)                 /* .text sections (code) */
    *(.text*)                /* .text* sections (code) */
    *(.rodata)               /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)              /* .rodata* sections (constants, strings, etc.) */
    *(CodeQuickAccess)       /* quick access code section */
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
  } > m_text

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > m_text

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > m_text

 .ctors :
  {
    __CTOR_LIST__ = .;
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    __CTOR_END__ = .;
  } > m_text

  .dtors :
  {
    __DTOR_LIST__ = .;
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    __DTOR_END__ = .;
  } > m_text

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } > m_text

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } > m_text

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > m_text

  __etext = .;    /* define a global symbol at end of code */
  __DATA_ROM = .; /* Symbol is used by startup for data initialization */

  .interrupts_ram :
  {
    . = ALIGN(4);
    __VECTOR_RAM__ = .;
    __interrupts_ram_start__ = .; /* Create a global symbol at data start */
    *(.m_interrupts_ram)     /* This is a user defined section */
    . += VECTOR_RAM_SIZE;
    . = ALIGN(4);
    __interrupts_ram_end__ = .; /* Define a global symbol at data end */
  } > m_interrupts_ram

  __VECTOR_RAM = DEFINED(__ram_vector_table__) ? __VECTOR_RAM__ : ORIGIN(m_interrupts);
  __RAM_VECTOR_TABLE_SIZE_BYTES = DEFINED(__ram_vector_table__) ? (__interrupts_ram_end__ - __interrupts_ram_start__) : 0x0;

  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __DATA_RAM = .;
    __data_start__ = .;      /* create a global symbol at data start */
    *(m_usb_dma_init_data)
    *(.data)                 /* .data sections */
    *(.data*)                /* .data* sections */
    *(DataQuickAccess)       /* quick access data section */
    KEEP(*(.jcr*))
    . = ALIGN(4);
    __data_end__ = .;        /* define a global symbol at data end */
  } > m_data

  __NDATA_ROM = __DATA_ROM + (__data_end__ - __data_start__); /* Symbol is used by startup for TCM data initialization */
  .ncache.init : AT(__NDATA_ROM)
  {
    __noncachedata_start__ = .;   /* create a global symbol at ncache data start */
    *(NonCacheable.init)
    . = ALIGN(4);
    __noncachedata_init_end__ = .;   /* create a global symbol at initialized ncache data end */
  } > m_data
  . = __noncachedata_init_end__;
  .ncache :
  {
    *(NonCacheable)
    . = ALIGN(4);
    __noncachedata_end__ = .;     /* define a global symbol at ncache data end */
  } > m_data

  __DATA_END = __NDATA_ROM + (__noncachedata_init_end__ - __noncachedata_start__);
  text_end = ORIGIN(m_text) + LENGTH(m_text);
  ASSERT(__DATA_END <= text_end, "region m_text overflowed with text and data")

  /* Uninitialized data section */
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    . = ALIGN(4);
    __START_BSS = .;
    __bss_start__ = .;
    *(m_usb_dma_noninit_data)
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    __END_BSS = .;
  } > m_data

  .heap :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    __HeapBase = .;
    . += HEAP_SIZE;
    __HeapLimit = .;
    __heap_limit = .; /* Add for _sbrk */
  } > m_heap

  .stack :
  {
    . = ALIGN(8);
    __StackStart = .;
    . += STACK_SIZE;
  } > m_data

  /* Initializes stack on the end of block */
  __StackTop   = ORIGIN(m_data) + LENGTH(m_data);
  __StackLimit = __StackTop - STACK_SIZE;
  PROVIDE(__stack = __StackTop);

  .ARM.attributes 0 : { *(.ARM.attributes) }

  ASSERT(__StackLimit >= __StackStart, "region m_data overflowed with stack and heap")
}
