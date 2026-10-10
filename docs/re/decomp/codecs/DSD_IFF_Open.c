/**
 * DSD_IFF_Open @ 0x030ff2e8
 * Tags: codec, dsd
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* Parse DSF (DSD Stream File): magic 'DSD ', then 'fmt ' (format id, channels, rate, bits), then

   'data' chunk. Returns 1 on success. Logs DSD err / formatid err / dsf read ckid err. */



undefined4 DSD_IFF_Open(void)



{

  char cVar1;

  undefined4 *puVar2;

  undefined4 *puVar3;

  uint uVar4;

  int iVar5;

  undefined4 uVar6;

  bool bVar7;

  bool bVar8;

  undefined4 uStack_50;

  byte local_48;

  char local_47;

  char local_46;

  char local_45;

  int iStack_44;

  undefined4 uStack_3c;

  undefined4 auStack_38 [2];

  undefined4 auStack_30 [2];

  undefined4 uStack_28;

  undefined4 uStack_24;

  undefined4 uStack_20;

  undefined4 uStack_1c;

  uint uStack_18;

  int aiStack_14 [2];

  

  puVar3 = DAT_030ff6d8;

  puVar2 = DAT_030ff6d4;

  uVar4 = (*(code *)*DAT_030ff6d8)(&local_48,0xc,*DAT_030ff6d4);

  bVar7 = uVar4 == 0xc;

  if (bVar7) {

    uVar4 = (uint)local_48;

  }

  if (bVar7 && uVar4 == 0x44) {

    bVar7 = local_47 == 'S';

    cVar1 = local_47;

    if (bVar7) {

      cVar1 = local_46;

    }

    bVar8 = bVar7 && cVar1 == 'D';

    if (bVar7 && cVar1 == 'D') {

      bVar8 = local_45 == ' ';

    }

    if (bVar8) {

      iVar5 = (*(code *)*puVar3)(auStack_38,8,*puVar2);

      if (iVar5 == 8) {

        puVar2[8] = auStack_38[0];

        FUN_030fce44(s_fileSize__d_030ff6e8);

      }

      iVar5 = (*(code *)*puVar3)(&uStack_50,8,*puVar2);

      if ((iVar5 != 8) || ((uStack_50 & 0xffffff) != 0 || uStack_50._3_1_ != 0)) {

        FUN_030fce44(s_ID3V2_start__d_030ff6f8,uStack_50 & 0xffffff | (uint)uStack_50._3_1_ << 0x18)

        ;

      }

      uVar4 = (*(code *)*puVar3)(&local_48,0xc,*puVar2);

      bVar7 = uVar4 == 0xc;

      if (bVar7) {

        uVar4 = (uint)local_48;

      }

      if (bVar7 && uVar4 == 0x66) {

        bVar7 = local_47 == 'm';

        cVar1 = local_47;

        if (bVar7) {

          cVar1 = local_46;

        }

        bVar8 = bVar7 && cVar1 == 't';

        if (bVar7 && cVar1 == 't') {

          bVar8 = local_45 == ' ';

        }

        if (bVar8) {

          iVar5 = (*(code *)*puVar3)(&uStack_3c,4,*puVar2);

          if (iVar5 == 4) {

            puVar2[1] = uStack_3c;

          }

          iVar5 = (*(code *)*puVar3)(&uStack_28,4,*puVar2);

          if ((iVar5 != 4) || ((uStack_28 & 0xffffff) != 0 || uStack_28._3_1_ != '\0')) {

            FUN_030fce44(s_formatid_err_030ff714);

            return 0;

          }

          iVar5 = (*(code *)*puVar3)(&uStack_24,4,*puVar2);

          if (iVar5 == 4) {

            puVar2[5] = uStack_24;

          }

          iVar5 = (*(code *)*puVar3)(&uStack_20,4,*puVar2);

          if (iVar5 == 4) {

            puVar2[4] = uStack_20;

          }

          iVar5 = (*(code *)*puVar3)(&uStack_1c,4,*puVar2);

          if (iVar5 == 4) {

            puVar2[2] = uStack_1c;

          }

          iVar5 = (*(code *)*puVar3)(&uStack_18,4,*puVar2);

          if ((iVar5 == 4) && (puVar2[3] = uStack_18, 7 < uStack_18)) {

            FUN_030fce44(s_bit_per_sample_not_support__030ff724);

            return 0;

          }

          iVar5 = (*(code *)*puVar3)(auStack_30,8,*puVar2);

          if (iVar5 == 8) {

            FUN_030fce44(s_sampleCount__lu_030ff744,auStack_30[0]);

          }

          iVar5 = (*(code *)*puVar3)(aiStack_14,4,*puVar2);

          if (iVar5 == 4) {

            puVar2[10] = puVar2[4] * aiStack_14[0];

          }

          (*(code *)*puRam030ff758)(4,1,*puVar2);

          uVar4 = (*(code *)*puVar3)(&local_48,0xc,*puVar2);

          bVar7 = uVar4 == 0xc;

          if (bVar7) {

            uVar4 = (uint)local_48;

          }

          if (bVar7 && uVar4 == 100) {

            bVar7 = local_47 == 'a';

            if (bVar7) {

              local_47 = local_46;

            }

            bVar8 = bVar7 && local_47 == 't';

            if (bVar7 && local_47 == 't') {

              bVar8 = local_45 == 'a';

            }

            if (bVar8) {

              uVar6 = (*(code *)*puRam030ff770)(*puVar2);

              puVar2[6] = uVar6;

              puVar2[7] = iStack_44 - 0xcU;

              puVar2[0xb] = (iStack_44 - 0xcU) / (uint)puVar2[10];

              FUN_030fce44(s_data_size____d_030ff774);

              return 1;

            }

          }

          FUN_030fce44(s_dsf_read_ckid_err_030ff75c);

          return 0;

        }

      }

      FUN_030fce44(s_fmt_err_030ff708);

      return 0;

    }

  }

  FUN_030fce44(s_DSD_err_030ff6dc);

  return 0;

}
