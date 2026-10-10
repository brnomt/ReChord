/**
 * ape_helper_a @ 0x030f7548
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 ape_helper_a(void)



{

  byte bVar1;

  undefined4 *puVar2;

  undefined4 *puVar3;

  undefined4 *puVar4;

  int iVar5;

  uint uVar6;

  undefined4 uVar7;

  uint uVar8;

  uint uVar9;

  bool bVar10;

  bool bVar11;

  undefined8 uVar12;

  byte local_38;

  byte local_37;

  byte local_36;

  byte local_35;

  byte local_30;

  byte local_2f;

  ushort local_2e;

  byte local_2c;

  byte local_2b;

  byte local_2a;

  byte local_29;

  byte local_28;

  byte local_27;

  ushort local_26;

  ushort local_24 [2];

  

  puVar3 = DAT_030f791c;

  puVar2 = DAT_030f7918;

  iVar5 = (*(code *)*DAT_030f791c)(&local_38,0xc,*DAT_030f7918);

  if (iVar5 == 0xc) {

    bVar1 = local_38;

    if (local_38 == 0x46) {

      bVar1 = local_37;

    }

    if (local_38 == 0x46 && bVar1 == 0x52) {

      bVar1 = local_36;

      if (local_36 == 0x4d) {

        bVar1 = local_35;

      }

      if (local_36 == 0x4d && bVar1 == 0x38) {

        uVar6 = (*(code *)*puVar3)(&local_2c,4,*puVar2);

        puVar4 = DAT_030f7920;

        bVar10 = uVar6 == 4;

        if (bVar10) {

          uVar6 = (uint)local_2c;

        }

        if (bVar10 && uVar6 == 0x44) {

          bVar1 = local_2b;

          if (local_2b == 0x53) {

            bVar1 = local_2a;

          }

          if ((local_2b == 0x53 && bVar1 == 0x44) && (local_29 == 0x20)) {

LAB_030f75c6:

            uVar12 = (*(code *)*puVar3)(&local_38,0xc,*puVar2);

            uVar6 = (uint)((ulonglong)uVar12 >> 0x20);

            if ((int)uVar12 == 0xc) {

              if (local_38 == 0x46) {

                uVar6 = (uint)local_37;

              }

              if (local_38 == 0x46 && uVar6 == 0x56) {

                uVar6 = (uint)local_36;

                bVar10 = uVar6 == 0x45;

                if (bVar10) {

                  uVar6 = (uint)local_35;

                }

                if ((bVar10 && uVar6 == 0x52) &&

                   (uVar6 = (local_2e & 0xff) << 8 | (uint)(local_2e >> 8) | (uint)local_2f << 0x10

                            | (uint)local_30 << 0x18, uVar6 == 4)) {

                  iVar5 = (*(code *)*puVar3)(&local_28,4,*puVar2);

                  if (iVar5 != 4) {

                    return 0;

                  }

                  puVar2[1] = (local_26 & 0xff) << 8 | (uint)(local_26 >> 8) |

                              (uint)local_27 << 0x10 | (uint)local_28 << 0x18;

                  goto LAB_030f75c6;

                }

              }

              if (local_38 == 0x50) {

                uVar6 = (uint)local_37;

              }

              if (local_38 == 0x50 && uVar6 == 0x52) {

                uVar6 = (uint)local_36;

                bVar10 = uVar6 == 0x4f;

                if (bVar10) {

                  uVar6 = (uint)local_35;

                }

                if (!bVar10 || uVar6 != 0x50) goto LAB_030f7658;

                uVar6 = (*(code *)*puVar3)(&local_2c,4,*puVar2);

                bVar10 = uVar6 == 4;

                if (bVar10) {

                  uVar6 = (uint)local_2c;

                }

                if (!bVar10 || uVar6 != 0x53) {

                  return 0;

                }

                bVar10 = local_2b == 0x4e;

                bVar1 = local_2b;

                if (bVar10) {

                  bVar1 = local_2a;

                }

                bVar11 = bVar10 && bVar1 == 0x44;

                if (bVar10 && bVar1 == 0x44) {

                  bVar11 = local_29 == 0x20;

                }

                if (!bVar11) {

                  return 0;

                }

                uVar9 = 0;

                uVar6 = (local_2e & 0xff) << 8 | (uint)(local_2e >> 8) | (uint)local_2f << 0x10 |

                        (uint)local_30 << 0x18;

                if (uVar6 != 4) {

                  do {

                    uVar12 = (*(code *)*puVar3)(&local_38,0xc,*puVar2);

                    uVar8 = (uint)((ulonglong)uVar12 >> 0x20);

                    if ((int)uVar12 != 0xc) {

                      return 0;

                    }

                    if (local_38 == 0x46) {

                      uVar8 = (uint)local_37;

                    }

                    if (local_38 != 0x46 || uVar8 != 0x53) {

LAB_030f7700:

                      if (local_38 == 0x43) {

                        uVar8 = (uint)local_37;

                      }

                      if (local_38 == 0x43 && uVar8 == 0x48) {

                        bVar1 = local_36;

                        if (local_36 == 0x4e) {

                          bVar1 = local_35;

                        }

                        if (local_36 == 0x4e && bVar1 == 0x4c) {

                          iVar5 = (*(code *)*puVar3)(local_24,4,*puVar2);

                          if (iVar5 != 4) {

                            return 0;

                          }

                          *(ushort *)(puVar2 + 4) = local_24[0] << 8 | local_24[0] >> 8;

                          (*(code *)*puVar4)(((local_2e & 0xff) << 8 | (uint)(local_2e >> 8) |

                                              (uint)local_2f << 0x10 | (uint)local_30 << 0x18) - 4,1

                                             ,*puVar2);

                          goto LAB_030f7844;

                        }

                      }

                      bVar1 = local_38;

                      if (local_38 == 0x43) {

                        bVar1 = local_37;

                      }

                      if (local_38 == 0x43 && bVar1 == 0x4d) {

                        bVar1 = local_36;

                        if (local_36 == 0x50) {

                          bVar1 = local_35;

                        }

                        if (local_36 == 0x50 && bVar1 == 0x52) {

                          uVar12 = (*(code *)*puVar3)(&local_2c,4,*puVar2);

                          uVar8 = (uint)((ulonglong)uVar12 >> 0x20);

                          if ((int)uVar12 != 4) {

                            return 0;

                          }

                          if (local_2c == 0x44) {

                            uVar8 = (uint)local_2b;

                          }

                          if (local_2c == 0x44 && uVar8 == 0x53) {

                            bVar1 = local_2a;

                            if (local_2a == 0x44) {

                              bVar1 = local_29;

                            }

                            if (local_2a == 0x44 && bVar1 == 0x20) {

                              puVar2[5] = 0;

                            }

                          }

                          bVar1 = local_2c;

                          if (local_2c == 0x44) {

                            bVar1 = local_2b;

                          }

                          if (local_2c == 0x44 && bVar1 == 0x53) {

                            bVar1 = local_2a;

                            if (local_2a == 0x54) {

                              bVar1 = local_29;

                            }

                            if (local_2a == 0x54 && bVar1 == 0x20) {

                              puVar2[5] = 1;

                            }

                          }

                          uVar8 = (local_2e & 0xff) << 8 | (uint)(local_2e >> 8) |

                                  (uint)local_2f << 0x10 | (uint)local_30 << 0x18;

                          if (((local_2e & 0xff) << 8 | (uint)(local_2e >> 8) |

                               (uint)local_2f << 0x10 | (uint)local_30 << 0x18) == 0x13) {

                            (*(code *)*puVar4)(uVar8 - 3,1,*puVar2);

                          }

                          else {

                            (*(code *)*puVar4)(uVar8 - 4,1,*puVar2);

                          }

                          goto LAB_030f7844;

                        }

                      }

                      (*(code *)*puVar4)((local_2e & 0xff) << 8 | (uint)(local_2e >> 8) |

                                         (uint)local_2f << 0x10 | (uint)local_30 << 0x18,1,*puVar2);

                    }

                    else {

                      uVar8 = (uint)local_36;

                      bVar10 = uVar8 != 0x20;

                      if (!bVar10) {

                        uVar8 = (uint)local_35;

                      }

                      if ((bVar10 || uVar8 != 0x20) ||

                         (uVar8 = (local_2e & 0xff) << 8 | (uint)(local_2e >> 8) |

                                  (uint)local_2f << 0x10 | (uint)local_30 << 0x18, uVar8 != 4))

                      goto LAB_030f7700;

                      iVar5 = (*(code *)*puVar3)(&local_28,4,*puVar2);

                      if (iVar5 != 4) {

                        return 0;

                      }

                      puVar2[2] = (local_26 & 0xff) << 8 | (uint)(local_26 >> 8) |

                                  (uint)local_27 << 0x10 | (uint)local_28 << 0x18;

                    }

LAB_030f7844:

                    iVar5 = ((local_2e & 0xff) << 8 | (uint)(local_2e >> 8) | (uint)local_2f << 0x10

                            | (uint)local_30 << 0x18) + uVar9;

                    if (((local_2e & 0xff) << 8 | (uint)(local_2e >> 8) | (uint)local_2f << 0x10 |

                        (uint)local_30 << 0x18) == 0x13) {

                      uVar9 = iVar5 + 0xd;

                    }

                    else {

                      uVar9 = iVar5 + 0xc;

                    }

                  } while (uVar9 < uVar6 - 4);

                }

              }

              else {

LAB_030f7658:

                if (local_38 == 0x44) {

                  uVar6 = (uint)local_37;

                }

                if (local_38 == 0x44 && uVar6 == 0x53) {

                  bVar1 = local_36;

                  if (local_36 == 0x44) {

                    bVar1 = local_35;

                  }

                  if (local_36 == 0x44 && bVar1 == 0x20) {

                    uVar7 = (*(code *)*DAT_030f7924)(*puVar2);

                    puVar2[6] = uVar7;

                    uVar6 = (local_2e & 0xff) << 8 | (uint)(local_2e >> 8) | (uint)local_2f << 0x10

                            | (uint)local_30 << 0x18;

                    puVar2[7] = uVar6;

                    *(undefined2 *)(puVar2 + 8) = 0x4b;

                    puVar2[9] = 0x24c0;

                    puVar2[10] = uVar6 / 0x24c0;

                    return 1;

                  }

                }

                bVar1 = local_38;

                if (local_38 == 0x44) {

                  bVar1 = local_37;

                }

                if (local_38 == 0x44 && bVar1 == 0x53) {

                  bVar1 = local_36;

                  if (local_36 == 0x54) {

                    bVar1 = local_35;

                  }

                  if (local_36 == 0x54 && bVar1 == 0x20) {

                    return 1;

                  }

                }

                (*(code *)*puVar4)((local_2e & 0xff) << 8 | (uint)(local_2e >> 8) |

                                   (uint)local_2f << 0x10 | (uint)local_30 << 0x18,1,*puVar2);

              }

              goto LAB_030f75c6;

            }

          }

        }

      }

    }

  }

  return 0;

}
