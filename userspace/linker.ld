ENTRY(_start)

SECTIONS
{
    . = 0x8048000;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)    
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata*)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
    _HEAP_START = ABSOLUTE(.) ;
    _HEAP_END = 0xFFFFCFFF ;

}
