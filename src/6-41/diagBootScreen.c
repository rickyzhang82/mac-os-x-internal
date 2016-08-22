// diagBootScreen.c
   
#include "diagCommon.h"
   
int
main(int argc, char **argv)
{
    struct vc_info vc_info;
   
    if (diagCall_(dgBootScreen, &vc_info) < 0)
        exit(1);
   
    printf("%ldx%ld pixels, %ldx%ld characters, %ld-bit\n",
           vc_info.v_width, vc_info.v_height,
           vc_info.v_columns, vc_info.v_rows,
           vc_info.v_depth);
    printf("base address %#08lx, physical address %#08lx\n",
           vc_info.v_baseaddr, vc_info.v_physaddr);
    printf("%ld bytes used for display per row\n",
           vc_info.v_rowscanbytes);
   
    exit(0);
}
