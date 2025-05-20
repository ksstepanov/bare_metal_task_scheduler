ENTRY(Reset_Handler)
/* r- readable only, x - executable */
MEMORY
{
  FLASH (rwx):   ORIGIN = 0x08000000, LENGTH = 1024K
  SRAM (rx): ORIGIN = 0x20000000, LENGTH = 256K
}
_estack = ORIGIN(SRAM) + LENGTH(SRAM);
SECTIONS
{

  .text :
  {
    *(.isr_vector)
    *(.text)
    *(.text.*)		/* To merge all small sections introduced by standard library */
    *(.rodata)
    *(.rodata.*)
    . = ALIGN(4);      /* by default sections are not aligned and if one is finished at non-word aligned address, next will start just after it */
    _etext = .;        /* symbol '.' identifies current location. This is location in VMA (one which is just after first > in "}> FLASH AT> FLASH " */
  }> FLASH AT> FLASH   /* }> <VMA address> AT> <LMA address>. Since this section is not relocatable, they are the same */ 
                       /* > FLASH THis is also ok, when lma =vma */

  _load_addr_data = LOADADDR(.data); /* this is the start address of .data in FLASH, required for startup code to copy */
   
  .data :
  {
    _sdata = .;
    *(.data)
    . = ALIGN(4);
    _edata = .;
  }> SRAM AT> FLASH
  
  .bss :
  {
    _sbss = .;
    __bss_start__ = .;  /* this specific name required by nano C standard library */
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = .; /* this specific name required by nano C standard library */
  }> SRAM 
  end = .; /* this specific name required by nano C standard library. This is used for memory management function to locate end of heap */
  __end__ = .; /* this specific name required by rdimon-nano C standard library. This is used for memory management function to locate end of heap */
}
