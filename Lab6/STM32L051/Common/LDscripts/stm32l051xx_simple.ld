MEMORY
{
  flash : org = 0x08000000, len = 64k
  ram :   org = 0x20000000, len = 8k
}

ENTRY(ResetInit)

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))
        *(.after_vectors*)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);

        PROVIDE(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE(__init_array_end = .);

        . = ALIGN(4);
        _etext = .;
    } > flash

    .data :
    {
        . = ALIGN(4) ;
        _data = .;
        *(.data*)
        . = ALIGN(4) ;
        _edata = .;
    } > ram AT > flash

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > ram

    PROVIDE(_HeapStart = .);
    PROVIDE(_StackTop = ORIGIN(ram) + LENGTH(ram));
}
