ENTRY(_start)

SECTIONS
{
	. = 0x10000;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.init)    /* the _start routine goes here.*/
		*(.text)    /* the rest of the code.*/
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
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

}
