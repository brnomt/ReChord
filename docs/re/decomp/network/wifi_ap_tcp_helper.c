/**
 * wifi_ap_tcp_helper @ 0x0308bafc
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void wifi_ap_tcp_helper(int param_1,int *param_2,int param_3,int param_4,int param_5,int param_6)



{

  longlong lVar1;

  longlong lVar2;

  longlong lVar3;

  longlong lVar4;

  undefined4 uVar5;

  uint uVar6;

  int iVar7;

  int iVar8;

  int iVar9;

  int iVar10;

  int iVar11;

  int iVar12;

  int iVar13;

  int iVar14;

  

  lVar1 = (longlong)(param_6 - param_5) * (longlong)param_2[2];

  lVar2 = (longlong)(param_6 + param_5) * (longlong)param_2[3];

  lVar3 = (longlong)(param_6 + param_5) * (longlong)param_2[2];

  lVar4 = (longlong)(param_6 - param_5) * (longlong)param_2[3];

  iVar12 = ((uint)lVar2 >> 0x1e | (int)((ulonglong)lVar2 >> 0x20) << 2) +

           ((uint)lVar1 >> 0x1e | (int)((ulonglong)lVar1 >> 0x20) << 2);

  iVar11 = ((uint)lVar3 >> 0x1e | (int)((ulonglong)lVar3 >> 0x20) << 2) -

           ((uint)lVar4 >> 0x1e | (int)((ulonglong)lVar4 >> 0x20) << 2);

  uVar6 = (uint)((longlong)param_6 * (longlong)param_5) >> 0x1e |

          (int)((ulonglong)((longlong)param_6 * (longlong)param_5) >> 0x20) << 2;

  iVar7 = uVar6 * 2;

  iVar14 = ((uint)((longlong)param_5 * (longlong)param_5) >> 0x1e |

           (int)((ulonglong)((longlong)param_5 * (longlong)param_5) >> 0x20) << 2) * -2 + 0x40000000

  ;

  iVar8 = iVar14 + uVar6 * -2;

  lVar1 = (longlong)iVar8 * (longlong)*param_2;

  lVar2 = (longlong)(iVar14 + iVar7) * (longlong)param_2[1];

  iVar10 = ((uint)lVar1 >> 0x1e | (int)((ulonglong)lVar1 >> 0x20) << 2) +

           ((uint)lVar2 >> 0x1e | (int)((ulonglong)lVar2 >> 0x20) << 2);

  lVar1 = (longlong)(iVar14 + iVar7) * (longlong)*param_2;

  lVar2 = (longlong)iVar8 * (longlong)param_2[1];

  iVar9 = ((uint)lVar1 >> 0x1e | (int)((ulonglong)lVar1 >> 0x20) << 2) -

          ((uint)lVar2 >> 0x1e | (int)((ulonglong)lVar2 >> 0x20) << 2);

  iVar13 = ((uint)((longlong)param_5 * (longlong)iVar14) >> 0x1e |

           (int)((ulonglong)((longlong)param_5 * (longlong)iVar14) >> 0x20) << 2) +

           ((uint)((longlong)param_6 * (longlong)iVar7) >> 0x1e |

           (int)((ulonglong)((longlong)param_6 * (longlong)iVar7) >> 0x20) << 2);

  iVar8 = ((uint)((longlong)param_6 * (longlong)iVar14) >> 0x1e |

          (int)((ulonglong)((longlong)param_6 * (longlong)iVar14) >> 0x20) << 2) -

          ((uint)((longlong)param_5 * (longlong)iVar7) >> 0x1e |

          (int)((ulonglong)((longlong)param_5 * (longlong)iVar7) >> 0x20) << 2);

  iVar7 = iVar8 - iVar13;

  iVar8 = iVar8 + iVar13;

  lVar1 = (longlong)iVar7 * (longlong)param_2[4];

  lVar2 = (longlong)iVar8 * (longlong)param_2[5];

  lVar3 = (longlong)iVar8 * (longlong)param_2[4];

  iVar8 = ((uint)lVar2 >> 0x1e | (int)((ulonglong)lVar2 >> 0x20) << 2) +

          ((uint)lVar1 >> 0x1e | (int)((ulonglong)lVar1 >> 0x20) << 2);

  lVar1 = (longlong)iVar7 * (longlong)param_2[5];

  iVar7 = ((uint)lVar3 >> 0x1e | (int)((ulonglong)lVar3 >> 0x20) << 2) -

          ((uint)lVar1 >> 0x1e | (int)((ulonglong)lVar1 >> 0x20) << 2);

  iVar13 = param_2[6] - param_2[7];

  iVar14 = param_2[6] + param_2[7];

  uVar5 = USBWriteEp(iVar10 + iVar12 + iVar8 + iVar14 >> 1,iVar9 + iVar11 + iVar7 + iVar13 >> 1);

  *(undefined4 *)(param_3 + param_1 * 4) = uVar5;

  uVar5 = USBWriteEp(((iVar11 - iVar10) - iVar7) + iVar14 >> 1,

                     ((iVar9 + iVar12) - iVar8) - iVar13 >> 1);

  *(undefined4 *)(param_3 + (param_4 - param_1) * 4) = uVar5;

  uVar5 = USBWriteEp((iVar7 - (iVar10 + iVar11)) + iVar14 >> 1,

                     ((iVar12 - iVar9) - iVar8) + iVar13 >> 1);

  *(undefined4 *)(param_3 + (param_1 + param_4) * 4) = uVar5;

  uVar5 = USBWriteEp(((iVar11 - iVar9) + iVar7) - iVar13 >> 1,

                     ((iVar10 - iVar12) - iVar8) + iVar14 >> 1);

  *(undefined4 *)(param_3 + (param_4 * 2 - param_1) * 4) = uVar5;

  return;

}
