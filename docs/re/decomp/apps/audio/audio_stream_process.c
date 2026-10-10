/**
 * audio_stream_process @ 0x0300cd00
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 audio_stream_process(int param_1,uint param_2,uint param_3,uint param_4)



{

  byte bVar1;

  uint unaff_r4;

  uint unaff_r5;

  int in_r12;

  int iVar2;

  

  do {

    if ((((unaff_r4 != 0x80) || ((*(byte *)(in_r12 + 4) & 0xc0) != 0x80)) ||

        ((*(byte *)(in_r12 + 5) & 0xc0) != 0x80)) ||

       (((*(byte *)(in_r12 + 6) & 0xc0) != 0x80 || ((*(byte *)(in_r12 + 7) & 0xc0) != 0x80)))) {

      return 0;

    }

    iVar2 = 7;

    param_3 = param_3 + 1 & 0xff;

    while( true ) {

      while( true ) {

        while( true ) {

          while( true ) {

            while( true ) {

              while( true ) {

                do {

                  param_4 = param_4 + iVar2 + 1 & 0xffff;

                  iVar2 = 0;

                  if (param_2 <= param_4) {

                    return 0;

                  }

                  if (unaff_r5 <= param_3) {

                    return 1;

                  }

                  bVar1 = *(byte *)(param_1 + param_4);

                } while ((bVar1 & 0x80) == 0);

                if ((bVar1 & 0xc0) == 0x80) {

                  return 0;

                }

                if ((bVar1 & 0xe0) != 0xc0) break;

                if ((*(byte *)(param_1 + param_4 + 1) & 0xc0) != 0x80) {

                  return 0;

                }

                param_3 = param_3 + 1 & 0xff;

                iVar2 = 1;

              }

              if ((bVar1 & 0xf0) != 0xe0) break;

              if ((*(byte *)(param_1 + param_4 + 1) & 0xc0) != 0x80) {

                return 0;

              }

              if ((*(byte *)(param_1 + param_4 + 2) & 0xc0) != 0x80) {

                return 0;

              }

              param_3 = param_3 + 1 & 0xff;

              iVar2 = 2;

            }

            if ((bVar1 & 0xf8) != 0xf0) break;

            iVar2 = param_1 + param_4;

            if ((*(byte *)(iVar2 + 1) & 0xc0) != 0x80) {

              return 0;

            }

            if ((*(byte *)(iVar2 + 2) & 0xc0) != 0x80) {

              return 0;

            }

            if ((*(byte *)(iVar2 + 3) & 0xc0) != 0x80) {

              return 0;

            }

            iVar2 = 3;

            param_3 = param_3 + 1 & 0xff;

          }

          if ((bVar1 & 0xfc) != 0xf8) break;

          iVar2 = param_1 + param_4;

          if ((*(byte *)(iVar2 + 1) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar2 + 2) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar2 + 3) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar2 + 4) & 0xc0) != 0x80) {

            return 0;

          }

          iVar2 = 4;

          param_3 = param_3 + 1 & 0xff;

        }

        if ((bVar1 & 0xfe) != 0xfc) break;

        iVar2 = param_1 + param_4;

        if ((*(byte *)(iVar2 + 1) & 0xc0) != 0x80) {

          return 0;

        }

        if ((*(byte *)(iVar2 + 2) & 0xc0) != 0x80) {

          return 0;

        }

        if ((*(byte *)(iVar2 + 3) & 0xc0) != 0x80) {

          return 0;

        }

        if ((*(byte *)(iVar2 + 4) & 0xc0) != 0x80) {

          return 0;

        }

        if ((*(byte *)(iVar2 + 5) & 0xc0) != 0x80) {

          return 0;

        }

        iVar2 = 5;

        param_3 = param_3 + 1 & 0xff;

      }

      if (bVar1 != 0xfe) break;

      iVar2 = param_1 + param_4;

      if ((*(byte *)(iVar2 + 1) & 0xc0) != 0x80) {

        return 0;

      }

      if ((*(byte *)(iVar2 + 2) & 0xc0) != 0x80) {

        return 0;

      }

      if ((*(byte *)(iVar2 + 3) & 0xc0) != 0x80) {

        return 0;

      }

      if ((*(byte *)(iVar2 + 4) & 0xc0) != 0x80) {

        return 0;

      }

      if ((*(byte *)(iVar2 + 5) & 0xc0) != 0x80) {

        return 0;

      }

      if ((*(byte *)(iVar2 + 6) & 0xc0) != 0x80) {

        return 0;

      }

      iVar2 = 6;

      param_3 = param_3 + 1 & 0xff;

    }

    if (bVar1 != 0xff) {

      return 0;

    }

    in_r12 = param_1 + param_4;

    if ((*(byte *)(in_r12 + 1) & 0xc0) != 0x80) {

      return 0;

    }

    if ((*(byte *)(in_r12 + 2) & 0xc0) != 0x80) {

      return 0;

    }

    unaff_r4 = *(byte *)(in_r12 + 3) & 0xc0;

  } while( true );

}
