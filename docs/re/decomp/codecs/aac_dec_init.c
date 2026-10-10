/**
 * aac_dec_init @ 0x030af458
 * Tags: codec, aac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



undefined4 aac_dec_init(int param_1,int param_2,uint param_3)



{

  int iVar1;

  int iVar2;

  int iVar3;

  uint uVar4;

  undefined4 *puVar5;

  int iVar6;

  int iVar7;

  undefined4 *puStack_44;

  uint local_40 [2];

  uint uStack_38;

  undefined4 *puStack_34;

  uint uStack_30;

  int local_2c [2];

  

  iVar3 = DAT_030af670;

  iVar2 = DAT_030af66c;

  iVar1 = DAT_030af644;

  local_40[0] = 0;

  iVar7 = 0;

  local_2c[0] = 0;

  if ((param_1 != 0) && (param_2 == 0)) {

    return 0xffffffff;

  }

  if (8 < (int)param_3) {

    do {

      uVar4 = buffered_fread(local_40,1,4,param_1);

      if (uVar4 < 4) {

        hifi_debug_printf_ovl

                  (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x4e7)

        ;

        return 0xfffffffb;

      }

      local_40[0] = local_40[0] << 0x18 | (local_40[0] >> 8 & 0xff) << 0x10 |

                    (local_40[0] >> 0x10 & 0xff) << 8 | local_40[0] >> 0x18;

      iVar7 = iVar7 + local_40[0];

      uVar4 = buffered_fread(local_2c,1,4,param_1);

      if (uVar4 < 4) {

        hifi_debug_printf_ovl

                  (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x4f0)

        ;

        return 0xfffffffb;

      }

      iVar6 = local_2c[0] - iVar2;

      if (local_2c[0] == iVar2) {

        uStack_38 = 0;

        uStack_30 = local_40[0];

        if ((param_1 != 0) && (param_2 == 0)) {

LAB_030af7e6:

          hifi_debug_printf_ovl

                    (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                     0x52d);

          return 0xffffffff;

        }

        buffered_fseek(param_1,4,1);

        uVar4 = buffered_fread(&uStack_38,1,4,param_1);

        puVar5 = DAT_030afb54;

        if (uVar4 < 4) goto LAB_030af7e6;

        uStack_38 = uStack_38 << 0x18 | (uStack_38 >> 8 & 0xff) << 0x10 |

                    (uStack_38 >> 0x10 & 0xff) << 8 | uStack_38 >> 0x18;

        if (*(int *)(iVar1 + 0x24) == 0) {

          *(uint *)(param_2 + 0x78) = uStack_38;

          iVar6 = (*(code *)*puVar5)(*(undefined4 *)(param_1 + 0x204));

          iVar6 = iVar6 - ((uint)*(ushort *)(param_1 + 0x202) - (uint)*(ushort *)(param_1 + 0x200));

          puStack_34 = *(undefined4 **)(iVar1 + 0x38);

          if (puStack_34 != (undefined4 *)0x0) {

            puStack_34[1] = iVar6;

            buffered_fseek(*puStack_34,iVar6,0);

            puVar5 = puStack_34;

LAB_030af7e2:

            puVar5[2] = 0;

          }

        }

        else {

          if (*(int *)(iVar1 + 0x24) != 1) {

LAB_030af760:

            buffered_fseek(param_1,uStack_30 - 0x10,1);

            goto LAB_030af53c;

          }

          *(uint *)(param_2 + 0x74) = uStack_38;

          iVar6 = (*(code *)*puVar5)(*(undefined4 *)(param_1 + 0x204));

          iVar6 = iVar6 - ((uint)*(ushort *)(param_1 + 0x202) - (uint)*(ushort *)(param_1 + 0x200));

          puStack_44 = *(undefined4 **)(iVar1 + 0x3c);

          if (puStack_44 != (undefined4 *)0x0) {

            puStack_44[1] = iVar6;

            buffered_fseek(*puStack_44,iVar6,0);

            puVar5 = puStack_44;

            goto LAB_030af7e2;

          }

        }

        buffered_fseek(param_1,uStack_38 << 2,1);

      }

      else if (local_2c[0] < iVar2) {

        if (local_2c[0] + iVar3 == 0) {

LAB_030af52c:

          buffered_fseek(param_1,local_40[0] - 8,1);

        }

        else {

          iVar6 = local_2c[0] + iVar3 + DAT_030af674;

          if (iVar6 == 0) {

            uStack_38 = 0;

            uStack_30 = local_40[0];

            if ((param_1 != 0) && (param_2 == 0)) {

LAB_030af6e8:

              hifi_debug_printf_ovl

                        (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                         0x513);

              return 0xffffffff;

            }

            buffered_fseek(param_1,4,1);

            uVar4 = buffered_fread(&uStack_38,1,4,param_1);

            if (uVar4 < 4) goto LAB_030af6e8;

            uStack_38 = uStack_38 << 0x18 | (uStack_38 >> 8 & 0xff) << 0x10 |

                        (uStack_38 >> 0x10 & 0xff) << 8 | uStack_38 >> 0x18;

            if (*(int *)(iVar1 + 0x24) == 0) {

              *(uint *)(param_2 + 0x8c) = uStack_38;

              iVar6 = FUN_030ae552(uStack_38 * 0xc);

              *(int *)(iVar1 + 0x30) = iVar6;

              if ((iVar6 == 0) ||

                 (uVar4 = buffered_fread(iVar6,1,uStack_38 * 0xc,param_1), uVar4 < uStack_38 * 0xc))

              goto LAB_030af6e8;

            }

            else {

              if (*(int *)(iVar1 + 0x24) != 1) goto LAB_030af760;

              *(uint *)(param_2 + 0x90) = uStack_38;

              iVar6 = FUN_030ae552(uStack_38 * 0xc);

              *(int *)(iVar1 + 0x34) = iVar6;

              if ((iVar6 == 0) ||

                 (uVar4 = buffered_fread(iVar6,1,uStack_38 * 0xc,param_1), uVar4 < uStack_38 * 0xc))

              goto LAB_030af6e8;

            }

          }

          else {

            if (iVar6 != 0x1000000) goto LAB_030af52c;

            iVar6 = FUN_030aeb84(param_1,param_2,local_40[0]);

            if (iVar6 != 0) {

              hifi_debug_printf_ovl

                        (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                         0x4f9);

              return 0xffffffff;

            }

          }

        }

      }

      else if (iVar6 == 0x4100000) {

        puStack_44 = (undefined4 *)0x0;

        if ((param_1 != 0) && (param_2 == 0)) goto LAB_030af896;

        if (*(int *)(iVar1 + 0x24) == 0) {

          buffered_fseek(param_1,4,1);

          uVar4 = buffered_fread(&puStack_44,1,4,param_1);

          if (uVar4 < 4) {

            hifi_debug_printf_ovl(DAT_030afb58 + -0x18,DAT_030afb58,0x49d);

            goto LAB_030af896;

          }

          puStack_44 = (undefined4 *)

                       ((int)puStack_44 << 0x18 | ((uint)puStack_44 >> 8 & 0xff) << 0x10 |

                        ((uint)puStack_44 >> 0x10 & 0xff) << 8 | (uint)puStack_44 >> 0x18);

          *(undefined4 **)(param_2 + 0x9c) = puStack_44;

          if (0 < (int)puStack_44) {

            iVar6 = FUN_030ae552((int)puStack_44 << 2);

            *(int *)(iVar1 + 0x48) = iVar6;

            if (iVar6 == 0) {

              hifi_debug_printf_ovl(DAT_030afb5c);

LAB_030af896:

              hifi_debug_printf_ovl

                        (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                         0x53e);

              return 0xffffffff;

            }

            uVar4 = buffered_fread(iVar6,1,*(int *)(param_2 + 0x9c) << 2,param_1);

            if (uVar4 < (uint)(*(int *)(param_2 + 0x9c) << 2)) {

              hifi_debug_printf_ovl(DAT_030afb58 + -0x18,DAT_030afb58,0x4b0);

              goto LAB_030af896;

            }

          }

        }

        else {

          buffered_fseek(param_1,local_40[0] - 8,1);

        }

      }

      else if (iVar6 == 0x4110000) {

        uStack_38 = 0;

        uStack_30 = local_40[0];

        if ((param_1 != 0) && (param_2 == 0)) {

LAB_030af5f6:

          hifi_debug_printf_ovl

                    (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                     0x502);

          return 0xffffffff;

        }

        buffered_fseek(param_1,4,1);

        uVar4 = buffered_fread(&uStack_38,1,4,param_1);

        if (uVar4 < 4) goto LAB_030af5f6;

        uStack_38 = uStack_38 << 0x18 | (uStack_38 >> 8 & 0xff) << 0x10 |

                    (uStack_38 >> 0x10 & 0xff) << 8 | uStack_38 >> 0x18;

        if (*(int *)(iVar1 + 0x24) == 0) {

          *(uint *)(param_2 + 0x98) = uStack_38;

          iVar6 = FUN_030ae552(uStack_38 << 3);

          *(int *)(iVar1 + 0x40) = iVar6;

          if ((iVar6 == 0) ||

             (uVar4 = buffered_fread(iVar6,1,uStack_38 << 3,param_1), uVar4 < uStack_38 << 3))

          goto LAB_030af5f6;

        }

        else {

          if (*(int *)(iVar1 + 0x24) != 1) goto LAB_030af760;

          *(uint *)(param_2 + 0x94) = uStack_38;

          iVar6 = FUN_030ae552(uStack_38 << 3);

          *(int *)(iVar1 + 0x44) = iVar6;

          if ((iVar6 == 0) ||

             (uVar4 = buffered_fread(iVar6,1,uStack_38 << 3,param_1), uVar4 < uStack_38 << 3))

          goto LAB_030af5f6;

        }

      }

      else {

        if (iVar6 != 0xb100000) goto LAB_030af52c;

        iVar6 = AudioFileOpen(param_1,param_2,local_40[0]);

        if (iVar6 != 0) {

          hifi_debug_printf_ovl

                    (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,

                     0x51c);

          return 0xffffffff;

        }

      }

LAB_030af53c:

    } while (iVar7 + 8U < param_3);

  }

  return 0;

}
