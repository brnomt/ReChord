/**
 * usb_probe_helper @ 0x0302850a
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 usb_probe_helper(void)



{

  int iVar1;

  int iVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  int iVar6;

  int iVar7;

  int iVar8;

  int iVar9;

  int iVar10;

  int iVar11;

  

  iVar3 = DAT_0302860c;

  iVar11 = DAT_03028608;

  iVar2 = DAT_030285f8;

  iVar1 = DAT_030285f4;

  iVar9 = DAT_030285e8;

  iVar10 = 0;

  while( true ) {

    if (*DAT_03028604 <= iVar10) {

      iVar10 = 0;

      do {

        if (iVar10 * 0x11 + iVar9 != 0) {

          iVar8 = *(int *)(iVar3 + iVar10 * 4);

          if (iVar8 == 0) {

            iVar8 = iVar11 + iVar10 * 0xd00;

            *(int *)(iVar3 + iVar10 * 4) = iVar8;

            if (iVar8 == 0) {

              return 0xffffff23;

            }

          }

          mbedtls_sha256_self_test(iVar10,iVar8);

        }

        iVar7 = DAT_03028620;

        iVar6 = DAT_0302861c;

        iVar5 = DAT_03028618;

        iVar4 = DAT_03028614;

        iVar8 = DAT_03028610;

        iVar10 = iVar10 + 1;

      } while (iVar10 < 8);

      iVar11 = *DAT_03028624;

      for (iVar9 = 0; iVar9 < iVar11; iVar9 = iVar9 + 1) {

        iVar10 = *(int *)(iVar8 + iVar9 * 4);

        *(undefined4 *)(iVar4 + iVar9 * 4) =

             *(undefined4 *)(iVar3 + *(int *)(iVar1 + iVar10 * 4) * 4);

        *(undefined4 *)(iVar5 + iVar9 * 4) =

             *(undefined4 *)(iVar3 + *(int *)(iVar2 + iVar10 * 4) * 4);

        *(int *)(iVar7 + iVar9 * 4) = iVar6 + iVar10 * 4;

      }

      return 0;

    }

    if ((*DAT_030285fc == 0) &&

       (*(int *)(DAT_030285f4 + *(int *)(DAT_030285f0 + iVar10 * 4) * 4) * 0x11 + DAT_030285e8 == 0)

       ) break;

    if ((0 < *DAT_03028600) &&

       (*(int *)(DAT_030285f8 + *(int *)(DAT_030285f0 + iVar10 * 4) * 4) * 0x11 + DAT_030285e8 == 0)

       ) {

      return 0xffffff21;

    }

    iVar10 = iVar10 + 1;

  }

  return 0xffffff21;

}
