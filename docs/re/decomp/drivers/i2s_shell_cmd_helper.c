/**
 * i2s_shell_cmd_helper @ 0x0301ca0c
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 i2s_shell_cmd_helper(int param_1,int param_2)



{

  byte bVar1;

  undefined2 uVar2;

  uint uVar3;

  undefined4 uVar4;

  uint uVar5;

  uint uVar6;

  uint uVar7;

  uint uVar8;

  int local_198;

  int local_194;

  int local_190 [2];

  byte abStack_185 [193];

  byte local_c4 [64];

  byte local_84 [32];

  byte abStack_64 [32];

  int local_44;

  uint local_40;

  uint local_3c;

  uint local_38;

  uint local_34;

  uint local_30;

  undefined4 local_2c;

  undefined4 local_28;

  

  local_194 = *(int *)(param_2 + 0x24);

  local_198 = *(int *)(param_2 + 0x24) + 0x900;

  if (*(char *)(param_2 + 1) == '\0') {

    uVar3 = 1;

  }

  else {

    uVar3 = 2;

  }

  if ((*(ushort *)(param_2 + 0x18) & 0x1000) != 0) {

    local_30 = 4;

    goto LAB_0301caac;

  }

  if ((*(ushort *)(param_2 + 0x18) & 0x400) == 0) {

    uVar5 = *(uint *)(param_2 + 0xc);

    if (uVar3 == 2) {

      uVar5 = uVar5 >> 1;

    }

    else if (DAT_0301ce50 < uVar5) {

      *(undefined2 *)(param_1 + 0x3c) = 0x222;

      return 0xffffffff;

    }

    if (uVar5 < 0xbb81) {

      if (*(int *)(param_2 + 0x10) == 32000) {

        local_30 = 3;

      }

      else {

        local_30 = 2;

      }

      goto LAB_0301caac;

    }

    if (uVar5 <= DAT_0301ce54) {

      local_30 = 0;

      goto LAB_0301caac;

    }

  }

  local_30 = (uint)(*(int *)(param_2 + 0x10) != 48000);

LAB_0301caac:

  local_34 = *(uint *)(*DAT_0301ce58 + local_30 * 0x24);

  local_44 = *DAT_0301ce58 + local_30 * 0x24 + 4;

  local_3c = 0x20;

  if (*(char *)(param_2 + 1) == '\x02') {

    *(uint *)(param_2 + 0x18) = *(uint *)(param_2 + 0x18) | 0x100;

    local_3c = *(int *)(param_2 + 4) * 4 + 4;

  }

  if (local_34 < local_3c) {

    local_3c = local_34;

  }

  local_28 = *(undefined4 *)(param_1 + 0x20);

  local_2c = *(undefined4 *)(param_1 + 0x1c);

  for (uVar5 = 0; uVar8 = local_3c, uVar5 < local_3c; uVar5 = uVar5 + 1) {

    local_38 = (uint)*(ushort *)(*DAT_0301ce5c + (uint)*(byte *)(local_44 + uVar5) * 4);

    for (uVar8 = 0; uVar8 < uVar3; uVar8 = uVar8 + 1) {

      bVar1 = bitstream_getbits_be_ovl_01c7(param_1 + 0x1c,local_38);

      local_84[uVar5 + uVar8 * 0x20] = bVar1;

    }

  }

  for (; uVar8 < local_34; uVar8 = uVar8 + 1) {

    local_38 = (uint)*(ushort *)(*DAT_0301ce5c + (uint)*(byte *)(local_44 + uVar8) * 4);

    bVar1 = bitstream_getbits_be_ovl_01c7(param_1 + 0x1c,local_38);

    abStack_64[uVar8] = bVar1;

    local_84[uVar8] = bVar1;

  }

  for (uVar5 = 0; uVar5 < local_34; uVar5 = uVar5 + 1) {

    for (uVar8 = 0; uVar8 < uVar3; uVar8 = uVar8 + 1) {

      if (local_84[uVar5 + uVar8 * 0x20] != 0) {

        bVar1 = bitstream_getbits_be_ovl_01c7(param_1 + 0x1c,2);

        local_c4[uVar5 + uVar8 * 0x20] = bVar1;

      }

    }

  }

  if ((*(byte *)(param_2 + 0x18) & 0x10) != 0) {

    uVar4 = FUN_0301c842(&local_2c,param_1 + 0x1c);

    uVar2 = USBWriteEp(local_2c,local_28,uVar4,*(undefined2 *)(param_2 + 0x14));

    *(undefined2 *)(param_2 + 0x14) = uVar2;

    if ((*(short *)(param_2 + 0x14) != *(short *)(param_2 + 0x16)) &&

       ((*(byte *)(param_2 + 0x20) & 1) == 0)) {

      *(undefined2 *)(param_1 + 0x3c) = 0x201;

      return 0xffffffff;

    }

  }

  uVar5 = 0;

  do {

    if (local_34 <= uVar5) {

      for (local_40 = 0; local_40 < 0xc; local_40 = local_40 + 1) {

        uVar8 = local_40 % 6;

        for (uVar5 = 0; uVar7 = local_3c, uVar5 < local_3c; uVar5 = uVar5 + 1) {

          for (uVar7 = 0; uVar7 < uVar3; uVar7 = uVar7 + 1) {

            local_30 = (uint)local_84[uVar5 + uVar7 * 0x20];

            if (local_30 == 0) {

              for (uVar6 = 0; uVar6 < 3; uVar6 = uVar6 + 1) {

                if (uVar7 == 0) {

                  *(undefined4 *)(local_194 + (uVar8 * 3 + uVar6) * 0x80 + uVar5 * 4) = 0;

                }

                else {

                  *(undefined4 *)(local_198 + (uVar8 * 3 + uVar6) * 0x80 + uVar5 * 4) = 0;

                }

              }

            }

            else {

              local_30 = (uint)*(byte *)(*DAT_0301ce60 +

                                         (uint)*(ushort *)

                                                (*DAT_0301ce5c +

                                                 (uint)*(byte *)(local_44 + uVar5) * 4 + 2) * 0xf +

                                        (local_30 - 1));

              DisplayDev_ClrRect(param_1 + 0x1c,*DAT_0301ce64 + local_30 * 0xc,local_190);

              for (uVar6 = 0; uVar6 < 3; uVar6 = uVar6 + 1) {

                if (uVar7 == 0) {

                  *(int *)(local_194 + (uVar8 * 3 + uVar6) * 0x80 + uVar5 * 4) =

                       (local_190[uVar6] >> 0xc) *

                       *(int *)(*DAT_0301ce68 +

                               (uint)abStack_185[(local_40 >> 2) + uVar5 * 3 + 1] * 4);

                }

                else {

                  *(int *)(local_198 + (uVar8 * 3 + uVar6) * 0x80 + uVar5 * 4) =

                       (local_190[uVar6] >> 0xc) *

                       *(int *)(*DAT_0301ce68 +

                               (uint)abStack_185[(local_40 >> 2) + uVar5 * 3 + uVar7 * 0x60 + 1] * 4

                               );

                }

              }

            }

          }

        }

        for (; uVar7 < local_34; uVar7 = uVar7 + 1) {

          local_30 = (uint)local_84[uVar7];

          if (local_30 == 0) {

            for (uVar5 = 0; uVar5 < uVar3; uVar5 = uVar5 + 1) {

              for (uVar6 = 0; uVar6 < 3; uVar6 = uVar6 + 1) {

                if (uVar5 == 0) {

                  *(undefined4 *)(local_194 + (uVar8 * 3 + uVar6) * 0x80 + uVar7 * 4) = 0;

                }

                else {

                  *(undefined4 *)(local_198 + (uVar8 * 3 + uVar6) * 0x80 + uVar7 * 4) = 0;

                }

              }

            }

          }

          else {

            local_30 = (uint)*(byte *)(*DAT_0301ce60 +

                                       (uint)*(ushort *)

                                              (*DAT_0301ce5c + (uint)*(byte *)(local_44 + uVar7) * 4

                                              + 2) * 0xf + (local_30 - 1));

            DisplayDev_ClrRect(param_1 + 0x1c,*DAT_0301ce64 + local_30 * 0xc,local_190);

            for (uVar5 = 0; uVar5 < uVar3; uVar5 = uVar5 + 1) {

              for (uVar6 = 0; uVar6 < 3; uVar6 = uVar6 + 1) {

                if (uVar5 == 0) {

                  *(int *)(local_194 + (uVar8 * 3 + uVar6) * 0x80 + uVar7 * 4) =

                       (local_190[uVar6] >> 0xc) *

                       *(int *)(*DAT_0301cff0 +

                               (uint)abStack_185[(local_40 >> 2) + uVar7 * 3 + 1] * 4);

                }

                else {

                  *(int *)(local_198 + (uVar8 * 3 + uVar6) * 0x80 + uVar7 * 4) =

                       (local_190[uVar6] >> 0xc) *

                       *(int *)(*DAT_0301cff0 +

                               (uint)abStack_185[(local_40 >> 2) + uVar7 * 3 + uVar5 * 0x60 + 1] * 4

                               );

                }

              }

            }

          }

        }

        for (uVar5 = 0; uVar5 < uVar3; uVar5 = uVar5 + 1) {

          for (uVar7 = 0; uVar6 = local_34, uVar7 < 3; uVar7 = uVar7 + 1) {

            for (; uVar6 < 0x20; uVar6 = uVar6 + 1) {

              if (uVar5 == 0) {

                *(undefined4 *)(local_194 + (uVar8 * 3 + uVar7) * 0x80 + uVar6 * 4) = 0;

              }

              else {

                *(undefined4 *)(local_198 + (uVar8 * 3 + uVar7) * 0x80 + uVar6 * 4) = 0;

              }

            }

          }

        }

        if (uVar8 == 5) {

          do {

          } while (*DAT_0301cff4 == 1);

          FUN_0301c7c6(param_2,0,0x12);

          if (1 < uVar3) {

            do {

            } while (*DAT_0301cff4 == 1);

            FUN_02ff5d5a(local_194,local_198,0x900);

            FUN_0301c7c6(param_2,1,0x12,0);

          }

          FUN_0301be38(param_2);

          local_194 = *(int *)(param_2 + 0x24);

          local_198 = *(int *)(param_2 + 0x24) + 0x900;

        }

      }

      return 0;

    }

    for (uVar8 = 0; uVar8 < uVar3; uVar8 = uVar8 + 1) {

      if (local_84[uVar5 + uVar8 * 0x20] != 0) {

        bVar1 = bitstream_getbits_be_ovl_01c7(param_1 + 0x1c,6);

        abStack_185[uVar5 * 3 + uVar8 * 0x60 + 1] = bVar1;

        bVar1 = local_c4[uVar5 + uVar8 * 0x20];

        if (bVar1 == 0) {

          bVar1 = bitstream_getbits_be_ovl_01c7(param_1 + 0x1c,6);

          abStack_185[uVar5 * 3 + uVar8 * 0x60 + 2] = bVar1;

LAB_0301cc72:

          bVar1 = bitstream_getbits_be_ovl_01c7(param_1 + 0x1c,6);

          abStack_185[uVar5 * 3 + uVar8 * 0x60 + 3] = bVar1;

        }

        else {

          if (bVar1 == 1) goto LAB_0301cc72;

          if (bVar1 == 2) {

            bVar1 = abStack_185[uVar5 * 3 + uVar8 * 0x60 + 1];

            abStack_185[uVar5 * 3 + uVar8 * 0x60 + 2] = bVar1;

            abStack_185[uVar5 * 3 + uVar8 * 0x60 + 3] = bVar1;

          }

          else if (bVar1 == 3) goto LAB_0301cc72;

        }

        if ((local_c4[uVar5 + uVar8 * 0x20] & 1) != 0) {

          abStack_185[uVar5 * 3 + uVar8 * 0x60 + 2] =

               abStack_185[(uint)local_c4[uVar5 + uVar8 * 0x20] + uVar5 * 3 + uVar8 * 0x60];

        }

      }

    }

    uVar5 = uVar5 + 1;

  } while( true );

}
