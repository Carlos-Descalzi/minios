ENTRY(module_init)
PHDRS
{
  headers PT_PHDR PHDRS ;
  text PT_LOAD FILEHDR PHDRS ;
  data PT_LOAD ;
  dynamic PT_DYNAMIC ;
}
SECTIONS
{

    . = SIZEOF_HEADERS;

	.text : /*BLOCK(4K) : ALIGN(4K)*/
	{
		*(.text)    
	}:text

	/*.got : {
	    *(.got)
	}:text*/

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}/*:text*/

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}/*:data*/

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}/*:data*/

}
