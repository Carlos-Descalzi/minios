BEGIN {
    print ";\n; Kernel link library for drivers\n;\n"
} 

{
    if ($2 == "T") 
        printf("global %s\n%s:\n\tjmp dword 0x8:0x%s\n",$3, $3, $1);
    else if ($2 == "D")
        printf("global %s\n%s: equ 0x%s\n",$3,$3,$1); 
}
