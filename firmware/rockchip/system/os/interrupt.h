/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   interrupt.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#undef  EXT
#ifdef _IN_INTTERUPT_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
//中断服务程序类型定义
typedef void(*ExecFunPtr)(void) __irq;

//section define
#define _ATTR_VECTTAB_              
#define _ATTR_INTRRUPT_CODE_        
#define _ATTR_INTRRUPT_DATA_        
#define _ATTR_INTRRUPT_BSS_         

//中断优先级定义
#define NUM_PRIORITY_BITS           4

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
//中断ID号定义
typedef enum
{
    //Cortex M3 System Intterupt
    FAULT_ID0_REV = (UINT32)0,                                  //SP Point
    FAULT_ID1_REV,                                              //Reset
    FAULT_ID2_NMI,                                              //NMI fault
    FAULT_ID3_HARD,                                             //Hard fault
    FAULT_ID4_MPU,                                              //MPU fault
    FAULT_ID5_BUS,                                              //Bus fault
    FAULT_ID6_USAGE,                                            //Usage fault
    FAULT_ID7_REV,                                              //
    FAULT_ID8_REV,                                              //
    FAULT_ID9_REV,                                              //
    FAULT_ID10_REV,                                             //
    FAULT_ID11_SVCALL,                                          //SVCall
    FAULT_ID12_DEBUG,                                           //Debug monitor
    FAULT_ID13_REV,                                             //
    FAULT_ID14_PENDSV,                                          //PendSV
    FAULT_ID15_SYSTICK,                                         //System Tick

    //RKNano Process Intterupt
    INT_ID_SFC,
    INT_ID_SYNTH,
    INT_ID_EBC,
    INT_ID_EMMC,
    INT_ID_SDMMC,
    INT_ID_USBC,
    INT_ID_DMA,
    INT_ID_IMDCT,
    INT_ID_WDT,
    INT_ID_MAILBOX0,
    INT_ID_MAILBOX1,
    INT_ID_MAILBOX2,
    INT_ID_MAILBOX3,
    INT_ID_REV0,
    INT_ID_REV1,
    INT_ID_REV2,
    INT_ID_PWM1,
    INT_ID_PWM0,
    INT_ID_TIMER1,
    INT_ID_TIMER0,
    INT_ID_SRADC,
    INT_ID_UART5,
    INT_ID_UART4,
    INT_ID_UART3,
    INT_ID_UART2,
    INT_ID_UART1,
    INT_ID_UART0,
    INT_ID_SPI1,
    INT_ID_SPI0,
    INT_ID_I2C2,
    INT_ID_I2C1,
    INT_ID_I2C0,
    INT_ID_I2S1,
    INT_ID_I2S0,
    INT_ID_HIFI,
    INT_ID_PMU,
    INT_ID_GPIO2,
    INT_ID_GPIO1,
    INT_ID_GPIO0,
    INT_ID_VOP,
    INT_ID_DMA2,

    NUM_INTERRUPTS                                              //Interrupts Total Nums

} INTERRUPT_ID;

_ATTR_INTRRUPT_BSS_ EXT uint8 UserIsrDisableCount;
_ATTR_INTRRUPT_BSS_ EXT uint8 UserIsrRequestCount;

_ATTR_INTRRUPT_BSS_ EXT uint8 IntMasterCounter;
_ATTR_INTRRUPT_BSS_ EXT uint8 UsbIntCounter;

/*
*-------------------------------------------------------------------------------
*
*                           Functon Declaration
*
*-------------------------------------------------------------------------------
*/

extern void   IntMasterEnable(void);
extern void   IntMasterDisable(void);
extern void   FaultMasterEnable(void);
extern void   FaultMasterDisable(void);
extern void   IntPriorityGroupingSet(UINT32 ulBits);
extern UINT32 IntPriorityGroupingGet(void);
extern void   IntPrioritySet(UINT32 ulInterrupt, UINT32 ucPriority);
extern UINT32 IntPriorityGet(UINT32 ulInterrupt);
extern void   IntEnable(UINT32 ulInterrupt);
extern void   IntDisable(UINT32 ulInterrupt);
extern void   IntPendingSet(UINT32 ulInterrupt);
extern void   IntPendingClear(UINT32 ulInterrupt);
extern BOOL   IntISRPendingCheck(void);
extern UINT32 IntVectPendingGet(void);
extern UINT32 IntVectActiveGet(void);
extern uint32 IsrDisable(UINT32 ulInterrupt);
extern void   IsrEnable(UINT32 ulInterrupt, uint32 IsEnable);
/*
#else

typedef void   (*pIntMasterEnable)(void);
typedef void   (*pIntMasterDisable)(void);
typedef void   (*pFaultMasterEnable)(void);
typedef void   (*pFaultMasterDisable)(void);
typedef void   (*pIntPriorityGroupingSet)(UINT32 ulBits);
typedef UINT32 (*pIntPriorityGroupingGet)(void);
typedef void   (*pIntPrioritySet)(UINT32 ulInterrupt, UINT32 ucPriority);
typedef UINT32 (*pIntPriorityGet)(UINT32 ulInterrupt);
typedef void   (*pIntEnable)(UINT32 ulInterrupt);
typedef void   (*pIntDisable)(UINT32 ulInterrupt);
typedef void   (*pIntPendingSet)(UINT32 ulInterrupt);
typedef void   (*pIntPendingClear)(UINT32 ulInterrupt);
typedef BOOL   (*pIntISRPendingCheck)(void);
typedef UINT32 (*pIntVectPendingGet)(void);
typedef UINT32 (*pIntVectActiveGet)(void);
typedef uint32 (*pIsrDisable)(UINT32 ulInterrupt);
typedef void   (*pIsrEnable)(UINT32 ulInterrupt, uint32 IsEnable);

#define IntMasterEnable()                       (((pIntMasterEnable       )(Addr_IntMasterEnable       ))())
#define IntMasterDisable()                      (((pIntMasterDisable      )(Addr_IntMasterDisable      ))())
#define FaultMasterEnable()                     (((pFaultMasterEnable     )(Addr_FaultMasterEnable     ))())
#define FaultMasterDisable()                    (((pFaultMasterDisable    )(Addr_FaultMasterDisable    ))())
#define IntPriorityGroupingSet(ulBits)          (((pIntPriorityGroupingSet)(Addr_IntPriorityGroupingSet))(ulBits))
#define IntPriorityGroupingGet()                (((pIntPriorityGroupingGet)(Addr_IntPriorityGroupingGet))())
#define IntPrioritySet(ulInterrupt, ucPriority) (((pIntPrioritySet        )(Addr_IntPrioritySet        ))(ulInterrupt, ucPriority))
#define IntPriorityGet(ulInterrupt)             (((pIntPriorityGet        )(Addr_IntPriorityGet        ))(ulInterrupt))
#define IntEnable(ulInterrupt)                  (((pIntEnable             )(Addr_IntEnable             ))(ulInterrupt))
#define IntDisable(ulInterrupt)                 (((pIntDisable            )(Addr_IntDisable            ))(ulInterrupt))
#define IntPendingSet(ulInterrupt)              (((pIntPendingSet         )(Addr_IntPendingSet         ))(ulInterrupt))
#define IntPendingClear(ulInterrupt)            (((pIntPendingClear       )(Addr_IntPendingClear       ))(ulInterrupt))
#define IntISRPendingCheck()                    (((pIntISRPendingCheck    )(Addr_IntISRPendingCheck    ))())
#define IntVectPendingGet()                     (((pIntVectPendingGet     )(Addr_IntVectPendingGet     ))())
#define IntVectActiveGet()                      (((pIntVectActiveGet      )(Addr_IntVectActiveGet      ))())
#define IsrDisable(ulInterrupt)                 (((pIsrDisable            )(Addr_IsrDisable            ))(ulInterrupt))
#define IsrEnable(ulInterrupt, IsEnable)        (((pIsrEnable             )(Addr_IsrEnable             ))(ulInterrupt, IsEnable))

#endif
*/
extern __irq void IntDefaultHandler(void);
extern void IntRegister(UINT32 ulInterrupt, ExecFunPtr(*pfnHandler)(void));
extern void IntUnregister(UINT32 ulInterrupt);
extern uint32 UserIsrDisable(void);
extern void UserIsrEnable();
extern void UserIsrRequest(void);
extern uint32 IntMasterControlInit(void);
extern uint32 EnableIntMaster(void);
extern uint32 DisableIntMaster(void);
extern uint32 UsbIntControlInit(void);
extern uint32 UsbIntEnable(void);
extern uint32 UsbIntDisalbe(void);
extern uint32 UsbHostIntEnable(void);
extern uint32 UsbHostIntDisable(void);

/*
********************************************************************************
*
*                         End of interrupt.h
*
********************************************************************************
*/
#endif

