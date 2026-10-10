/**
 * aac_movfile_parser @ 0x030aeb90
 * Tags: codec, aac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int aac_movfile_parser(int param_1,int param_2,int param_3)



{

  int iVar1;

  short sVar2;

  ushort uVar3;

  int iVar4;

  int iVar5;

  undefined4 *puVar6;

  uint uVar7;

  int iVar8;

  int iVar9;

  bool bVar10;

  bool bVar11;

  uint local_50;

  int local_4c;

  uint local_48;

  int local_44;

  int local_40;

  int local_3c;

  undefined1 auStack_38 [4];

  uint local_34;

  uint local_30 [3];

  

  iVar4 = DAT_030aed98;

  if (*(int *)(DAT_030aed98 + 0x24) == 2) {

    buffered_fseek(param_1,param_3 + -8,1);

  }

  else {

    local_44 = 0;

    buffered_fseek(param_1,4,1);

    uVar7 = buffered_fread(&local_50,1,4,param_1);

    iVar5 = DAT_030aed9c;

    if (uVar7 < 4) {

      hifi_debug_printf_ovl

                (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1fd);

      return -5;

    }

    local_50 = (int)local_50 >> 8 & 0xff00U | local_50 >> 0x18 | (local_50 & 0xff00) << 8 |

               local_50 << 0x18;

    local_4c = 0;

    if (0 < (int)local_50) {

      do {

        uVar7 = buffered_fread(&local_48,1,4,param_1);

        if (uVar7 < 4) {

          return -5;

        }

        local_48 = (int)local_48 >> 8 & 0xff00U | local_48 >> 0x18 | (local_48 & 0xff00) << 8 |

                   local_48 << 0x18;

        uVar7 = buffered_fread(&local_44,1,4,param_1);

        iVar9 = DAT_030aedb4;

        if (uVar7 < 4) {

          return -5;

        }

        if (*(int *)(iVar4 + 0x24) == 0) {

          if (local_44 + DAT_030aedac == 0) {

            *(undefined2 *)(param_2 + 0xd2) = 3;

            FUN_030ac90c(iVar5 + 0x4f8,&DAT_030af1c0,4);

          }

          else {

            iVar8 = local_44 + DAT_030aedac + DAT_030aedb0;

            if (iVar8 == 0 || iVar8 == 0xb) {

              *(undefined2 *)(param_2 + 0xd2) = 2;

              FUN_030ac90c(iVar5 + 0x4f8,&DAT_030aedbc,4);

            }

            else {

              if (iVar8 + DAT_030aedb4 != 0) {

                *(undefined2 *)(param_2 + 0xd2) = 0;

                return iVar9 >> 0x1d;

              }

              *(undefined2 *)(param_2 + 0xd2) = 1;

              FUN_030ac90c(iVar5 + 0x4f8,&DAT_030aedb8,4);

            }

          }

          buffered_fseek(param_1,0x18,1);

          uVar7 = buffered_fread(param_2 + 0xcc,1,2,param_1);

          if (uVar7 < 2) {

            return -5;

          }

          *(ushort *)(param_2 + 0xcc) =

               *(ushort *)(param_2 + 0xcc) >> 8 | *(ushort *)(param_2 + 0xcc) << 8;

          uVar7 = buffered_fread(param_2 + 0xce,1,2,param_1);

          if (uVar7 < 2) {

            return -5;

          }

          uVar3 = *(ushort *)(param_2 + 0xce) >> 8 | *(ushort *)(param_2 + 0xce) << 8;

          iVar9 = (int)(short)uVar3;

          *(ushort *)(param_2 + 0xce) = uVar3;

          sVar2 = *(short *)(param_2 + 0xd2);

          iVar8 = (int)*(short *)(param_2 + 0xcc);

          if (sVar2 == 3) {

            bVar11 = SBORROW4(iVar8,0x780);

            iVar1 = iVar8 + -0x780;

            bVar10 = iVar8 == 0x780;

            if (iVar8 < 0x781) {

              bVar11 = SBORROW4(iVar9,0x438);

              iVar1 = iVar9 + -0x438;

              bVar10 = iVar9 == 0x438;

            }

            if (!bVar10 && iVar1 < 0 == bVar11) {

              hifi_debug_printf_ovl

                        (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                         0x23d);

              return -4;

            }

          }

          else {

            bVar11 = SBORROW4(iVar8,0x500);

            iVar1 = iVar8 + -0x500;

            bVar10 = iVar8 == 0x500;

            if (iVar8 < 0x501) {

              bVar11 = SBORROW4(iVar9,0x2d0);

              iVar1 = iVar9 + -0x2d0;

              bVar10 = iVar9 == 0x2d0;

            }

            if (!bVar10 && iVar1 < 0 == bVar11) {

              hifi_debug_printf_ovl

                        (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                         0x246);

              return -4;

            }

          }

          if (sVar2 == 1 || sVar2 == 3) {

            local_30[0] = 0;

            local_34 = 0;

            buffered_fseek(param_1,0x32,1);

            uVar7 = buffered_fread(local_30,1,4,param_1);

            if (uVar7 < 4) {

              return -5;

            }

            local_30[0] = local_30[0] << 0x18 | (local_30[0] >> 8 & 0xff) << 0x10 |

                          (local_30[0] >> 0x10 & 0xff) << 8 | local_30[0] >> 0x18;

            uVar7 = buffered_fread(&local_34,1,4,param_1);

            if (uVar7 < 4) {

              return -5;

            }

            if (local_34 == DAT_030af1c4) {

              buffered_fseek(param_1,4,1);

              puVar6 = DAT_030af1c8;

              *(uint *)(param_2 + 0x50) = local_30[0] - 0xc;

              iVar9 = (*(code *)*puVar6)(*(undefined4 *)(param_1 + 0x204));

              *(uint *)(param_2 + 0x4c) =

                   iVar9 - ((uint)*(ushort *)(param_1 + 0x202) - (uint)*(ushort *)(param_1 + 0x200))

              ;

              buffered_fseek(param_1,local_48 - 0x62,1);

            }

            else if (local_34 == DAT_030af1cc) {

              buffered_fseek(param_1,4,1);

              puVar6 = DAT_030af1c8;

              *(uint *)(param_2 + 0x5c) = local_30[0] - 0xc;

              iVar9 = (*(code *)*puVar6)(*(undefined4 *)(param_1 + 0x204));

              *(uint *)(param_2 + 0x58) =

                   iVar9 - ((uint)*(ushort *)(param_1 + 0x202) - (uint)*(ushort *)(param_1 + 0x200))

              ;

              buffered_fseek(param_1,5,1);

              iVar9 = buffered_fread(auStack_38,1,1,param_1);

              if (iVar9 == 0) {

                return -5;

              }

              buffered_fseek(param_1,local_48 - 0x68,1);

            }

            else {

              buffered_fseek(param_1,local_48 - 0x5e,1);

            }

          }

          else {

            buffered_fseek(param_1,local_48 - 0x24,1);

          }

        }

        else {

          if (local_44 + DAT_030aeda0 == 0) {

            *(undefined2 *)(param_2 + 0xd0) = 3;

            FUN_030ac90c(iVar5 + 0x4e8,&DAT_030af1d8,0x10);

          }

          else {

            iVar9 = local_44 + DAT_030aeda0 + DAT_030aeda4;

            if (iVar9 == 0) {

              *(undefined2 *)(param_2 + 0xd0) = 2;

              FUN_030ac90c(iVar5 + 0x4e8,&DAT_030af1d4,0x10);

            }

            else if (iVar9 + DAT_030aeda8 == 0) {

              *(undefined2 *)(param_2 + 0xd0) = 1;

              FUN_030ac90c(iVar5 + 0x4e8,&DAT_030af1d0,0x10);

            }

            else {

              *(undefined2 *)(param_2 + 0xd0) = 0;

            }

          }

          buffered_fseek(param_1,0x10,1);

          uVar7 = buffered_fread(param_2 + 0xc4,1,2,param_1);

          if (uVar7 < 2) {

            return -5;

          }

          *(ushort *)(param_2 + 0xc4) =

               *(ushort *)(param_2 + 0xc4) >> 8 | *(ushort *)(param_2 + 0xc4) << 8;

          buffered_fseek(param_1,6,1);

          uVar7 = buffered_fread(param_2 + 200,1,4,param_1);

          if (uVar7 < 4) {

            return -5;

          }

          *(uint *)(param_2 + 200) =

               (*(uint *)(param_2 + 200) << 0x18 | (*(uint *)(param_2 + 200) >> 8 & 0xff) << 0x10)

               >> 0x10;

          local_34 = 0;

          local_30[0] = 0;

          uVar7 = buffered_fread(&local_34,1,4,param_1);

          if (uVar7 < 4) {

            return -5;

          }

          local_34 = local_34 << 0x18 | (local_34 >> 8 & 0xff) << 0x10 |

                     (local_34 >> 0x10 & 0xff) << 8 | local_34 >> 0x18;

          uVar7 = buffered_fread(local_30,1,4,param_1);

          if (uVar7 < 4) {

            return -5;

          }

          if (local_30[0] == DAT_030af1c4) {

            iVar9 = aac_movfile_seek(param_1,&local_3c,&local_40);

            if (iVar9 != 0) {

              hifi_debug_printf_ovl

                        (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                         0x2ce);

              return -1;

            }

            if ((*(int *)(param_2 + 200) != local_3c) || (*(short *)(param_2 + 0xc4) != local_40)) {

              *(int *)(param_2 + 200) = local_3c;

              *(short *)(param_2 + 0xc4) = (short)local_40;

            }

          }

          buffered_fseek(param_1,local_48 - 0x2c,1);

        }

        local_4c = local_4c + 1;

      } while (local_4c < (int)local_50);

    }

  }

  return 0;

}
