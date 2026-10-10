/**
 * rom_gui_check_area @ 0x02ff7e0a
 * Tags: rom, ui
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Control flow encountered bad instruction data */

/* GUI area check: returns nonzero if current key/focus maps into LCD rect (x1,y1)-(x2,y2). Used for

   soft-keyboard cells and list rows. Not capacitive touch — Echo Mini is LADC button matrix only.

    */



void rom_gui_check_area(void)



{

                    /* WARNING: Bad instruction - Truncating control flow here */

  halt_baddata();

}
