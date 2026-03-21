ENTRY(loader)                /* the name of the entry label */

. = 0xC0100000; /* the code should be relocated to 3GB + 1MB LMA*/

SECTIONS {

    .multiboot ALIGN(4) :
    {
        *(.multiboot)
    }

    .text ALIGN (0x1000) : AT(ADDR(.text)-0xC0000000) /* align at 4 KB and load at 1 MB LMA*/
    {
        *(.text)             /* all text sections from all files */
    }

    .rodata ALIGN (0x1000) : AT(ADDR(.rodata)-0xC0000000) /* align at 4 KB and load at 1 MB LMA*/
    {
        *(.rodata*)          /* all read-only data sections from all files */
    }

    .data ALIGN (0x1000) : AT(ADDR(.data)-0xC0000000)  /* align at 4 KB and load at 1 MB LMA*/
    {
        *(.data)             /* all data sections from all files */
    }

    .bss ALIGN (0x1000) : AT(ADDR(.bss)-0xC0000000)  /* align at 4 KB and load at 1 MB LMA*/
    {
        *(COMMON)            /* all COMMON sections from all files */
        *(.bss)              /* all bss sections from all files */
    }
}