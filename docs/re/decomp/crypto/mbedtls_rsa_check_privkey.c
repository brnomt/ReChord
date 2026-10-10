/**
 * mbedtls_rsa_check_privkey @ 0x0301a87c
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Control flow encountered bad instruction data */



void mbedtls_rsa_check_privkey(void)



{

  int iVar1;

  

  iVar1 = FUN_02ff7f24(*DAT_0301a8b8,DAT_0301a8b4);

  if (iVar1 == 1) {

    return;

  }

  FUN_02ff7ee0(DAT_0301a8b8);

  mbedtls_mpi_div_mpi(0xe,7);

                    /* WARNING: Bad instruction - Truncating control flow here */

  halt_baddata();

}
