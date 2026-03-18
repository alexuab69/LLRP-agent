/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008-2013 Code Red Technologies Ltd,
 * Copyright 2015, 2018-2019 NXP
 * (c) NXP Semiconductors 2013-2020
 * Generated linker script file for MKL27Z64xxx4
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.23
 * MCUXpresso IDE v11.2.0 [Build 4120] [2020-07-09] on 15 Sep, 2020 11:54:18 AM
 */

MEMORY
{
  /* Define each memory region */
  PROGRAM_FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x10000 /* 64K bytes (alias Flash) */  
  SRAM (rwx) : ORIGIN = 0x1ffff000, LENGTH = 0x4000 /* 16K bytes (alias RAM) */  
  USB_RAM (rwx) : ORIGIN = 0x400fe000, LENGTH = 0x200 /* 512 bytes (alias RAM2) */  
}

  /* Define a symbol for the top of each memory region */
  __base_PROGRAM_FLASH = 0x0  ; /* PROGRAM_FLASH */  
  __base_Flash = 0x0 ; /* Flash */  
  __top_PROGRAM_FLASH = 0x0 + 0x10000 ; /* 64K bytes */  
  __top_Flash = 0x0 + 0x10000 ; /* 64K bytes */  
  __base_SRAM = 0x1ffff000  ; /* SRAM */  
  __base_RAM = 0x1ffff000 ; /* RAM */  
  __top_SRAM = 0x1ffff000 + 0x4000 ; /* 16K bytes */  
  __top_RAM = 0x1ffff000 + 0x4000 ; /* 16K bytes */  
  __base_USB_RAM = 0x400fe000  ; /* USB_RAM */  
  __base_RAM2 = 0x400fe000 ; /* RAM2 */  
  __top_USB_RAM = 0x400fe000 + 0x200 ; /* 512 bytes */  
  __top_RAM2 = 0x400fe000 + 0x200 ; /* 512 bytes */  
