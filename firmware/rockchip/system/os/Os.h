/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Os.h
* 
* Description:  
*                      
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _OS_H_
#define _OS_H_

/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/

extern void __WFI(void);
extern void __WFE(void);
extern void __SEV(void);
extern void __ISB(void);
extern void __DSB(void);
extern void __DMB(void);
extern void __SVC(void);
extern UINT32 __MRS_CONTROL(void);
extern void __MSR_CONTROL(UINT32 Control);
extern UINT32 __MRS_PSP(void);
extern void __MSR_PSP(UINT32 ProcStack);
extern UINT32 __MRS_MSP(void);
extern void __MSR_MSP(UINT32 MainStack);
extern void __SETPRIMASK(void);
extern void __RESETPRIMASK(void);
extern void __SETFAULTMASK(void);
extern void __RESETFAULTMASK(void);
extern void __BASEPRICONFIG(UINT32 NewPriority);
extern UINT32 __GetBASEPRI(void);
extern UINT16 __REV_HalfWord(UINT16 data);
extern UINT32 __REV_Word(UINT32 data);
/*
#else

typedef void         (*p__WFI)(void);
typedef void         (*p__WFE)(void);
typedef void         (*p__SEV)(void);
typedef void         (*p__ISB)(void);
typedef void         (*p__DSB)(void);
typedef void         (*p__DMB)(void);
typedef void         (*p__SVC)(void);
typedef UINT32       (*p__MRS_CONTROL)(void);
typedef void         (*p__MSR_CONTROL)(UINT32 Control);
typedef UINT32       (*p__MRS_PSP)(void);
typedef void         (*p__MSR_PSP)(UINT32 ProcStack);
typedef UINT32       (*p__MRS_MSP)(void);
typedef void         (*p__MSR_MSP)(UINT32 MainStack);
typedef void         (*p__SETPRIMASK)(void);
typedef void         (*p__RESETPRIMASK)(void);
typedef void         (*p__SETFAULTMASK)(void);
typedef void         (*p__RESETFAULTMASK)(void);
typedef void         (*p__BASEPRICONFIG)(UINT32 NewPriority);
typedef UINT32       (*p__GetBASEPRI)(void);
typedef UINT16       (*p__REV_HalfWord)(UINT16 data);
typedef UINT32       (*p__REV_Word)(UINT32 data);

#define    __WFI()                                                                              (((p__WFI                 )(Addr___WFI                     ))())
#define    __WFE()                                                                              (((p__WFE                 )(Addr___WFE                     ))())
#define    __SEV()                                                                              (((p__SEV                 )(Addr___SEV                     ))())
#define    __ISB()                                                                              (((p__ISB                 )(Addr___ISB                     ))())
#define    __DSB()                                                                              (((p__DSB                 )(Addr___DSB                     ))())
#define    __DMB()                                                                              (((p__DMB                 )(Addr___DMB                     ))())
#define    __SVC()                                                                              (((p__SVC                 )(Addr___SVC                     ))())
#define    __MRS_CONTROL()                                                                      (((p__MRS_CONTROL         )(Addr___MRS_CONTROL             ))())
#define    __MSR_CONTROL(Control)                                                               (((p__MSR_CONTROL         )(Addr___MSR_CONTROL             ))(Control))
#define    __MRS_PSP()                                                                          (((p__MRS_PSP             )(Addr___MRS_PSP                 ))())
#define    __MSR_PSP(ProcStack)                                                                 (((p__MSR_PSP             )(Addr___MSR_PSP                 ))(ProcStack))
#define    __MRS_MSP()                                                                          (((p__MRS_MSP             )(Addr___MRS_MSP                 ))())
#define    __MSR_MSP(MainStack)                                                                 (((p__MSR_MSP             )(Addr___MSR_MSP                 ))(MainStack))
#define    __SETPRIMASK()                                                                       (((p__SETPRIMASK          )(Addr___SETPRIMASK              ))())
#define    __RESETPRIMASK()                                                                     (((p__RESETPRIMASK        )(Addr___RESETPRIMASK            ))())
#define    __SETFAULTMASK()                                                                     (((p__SETFAULTMASK        )(Addr___SETFAULTMASK            ))())
#define    __RESETFAULTMASK()                                                                   (((p__RESETFAULTMASK      )(Addr___RESETFAULTMASK          ))())
#define    __BASEPRICONFIG(NewPriority)                                                         (((p__BASEPRICONFIG       )(Addr___BASEPRICONFIG           ))(NewPriority))
#define    __GetBASEPRI()                                                                       (((p__GetBASEPRI          )(Addr___GetBASEPRI              ))())
#define    __REV_HalfWord(data)                                                                 (((p__REV_HalfWord        )(Addr___REV_HalfWord            ))(data))
#define    __REV_Word(data)                                                                     (((p__REV_Word            )(Addr___REV_Word                ))(data))

#endif
*/
/*
********************************************************************************
*
*                         End of Os.h
*
********************************************************************************
*/
#endif
