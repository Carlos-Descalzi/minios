# This script parses the symbol table exported from kernel
# and generates an assembly file containing absolute jumps to kernel functions.
# This is later compiled as a library for loadable modules.
BEGIN {
    print ";\n; Kernel link library for loadable modules\n;\n"
} 

{
    if ($2 == "T") 
        printf("global %s\n%s:\n\tjmp dword 0x8:0x%s\n",$3, $3, $1);
    else if ($2 == "D")
        printf("global %s\n%s: equ 0x%s\n",$3,$3,$1); 
}
