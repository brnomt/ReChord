/**
 * phybusif_reset @ 0x03008fc4
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 phybusif_reset(void)



{

  undefined4 *puVar1;

  int iVar2;

  int iVar3;

  undefined2 local_30 [18];

  

  iVar2 = FUN_02ff0ea6();

  puVar1 = DAT_03009054;

  iVar3 = iVar2 - DAT_0300906c;

  if (iVar2 == DAT_0300906c) {

    iVar2 = FUN_02ff7414(1);

    if (iVar2 == 0) {

      AudioStart();

      return 0;

    }

    goto LAB_030090f8;

  }

  if (DAT_0300906c <= iVar2) {

    if (iVar3 == DAT_0300907c) {

      local_30[0] = 0;

      FUN_02fe62b0(DAT_03009490,DAT_03009494,local_30);

      return 0;

    }

    if (iVar3 < DAT_0300907c) {

      if (iVar3 != 0x1c0 && iVar3 != 0xfc0) {

        return 0;

      }

    }

    else {

      if (iVar3 - DAT_0300907c == 0x1000000) {

        local_30[0] = 1;

        FUN_02fe62b0(DAT_03009490,DAT_03009494,local_30);

        return 0;

      }

      if (iVar3 - DAT_0300907c != 0xf000200) {

        return 0;

      }

    }

LAB_0300910e:

    iVar2 = FUN_02ff7414(1);

    if (iVar2 == 0) {

      MBEDTLS_MPI_CHK(DAT_03009490);

      memset(0x21);

      mbedtls_mpi_grow(0x82);

      return 1;

    }

LAB_030090f8:

    mbedtls_mpi_grow(0x2f);

    memset(1);

    return 0;

  }

  if (iVar2 == DAT_03009070) {

LAB_030090b4:

    if (*(char *)(DAT_03009068 + 0x12) != '\0') {

      *(char *)(DAT_03009068 + 0x12) = *(char *)(DAT_03009068 + 0x12) + -1;

    }

    iVar2 = pbuf_alloc(*puVar1,DAT_03009488);

    if (iVar2 == 1) {

      phybusif_output(0xff,0);

    }

    iVar2 = pbuf_alloc(*puVar1,DAT_0300948c);

  }

  else {

    if (iVar2 < DAT_03009070) {

      if (iVar2 + -0x20001000 == 0) goto LAB_0300910e;

      if (iVar2 + -0x20001000 + DAT_03009074 != 0) {

        return 0;

      }

    }

    else {

      iVar2 = (iVar2 - DAT_03009070) + DAT_03009078;

      if (iVar2 != 0) {

        if (iVar2 != 0x10) {

          return 0;

        }

        goto LAB_030090b4;

      }

    }

    if (*(byte *)(DAT_03009068 + 0x12) < *(byte *)(DAT_03009068 + 0x1e)) {

      *(byte *)(DAT_03009068 + 0x12) = *(byte *)(DAT_03009068 + 0x12) + 1;

    }

    iVar2 = pbuf_alloc(*puVar1,DAT_03009488);

    if (iVar2 == 1) {

      phybusif_output(0xff,0);

    }

    iVar2 = pbuf_alloc(*puVar1,DAT_0300948c);

  }

  if (iVar2 == 1) {

    pbuf_free(0xb6,0);

  }

  mbedtls_mpi_grow(0x33);

  return 0;

}
