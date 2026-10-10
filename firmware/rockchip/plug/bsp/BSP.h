/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright 2013, RockChip.Ltd
*                                          All Rights Reserved
*File    : BSP.h
* By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/
#ifndef _BSP_H_
#define _BSP_H_

#include "global.h"
#include "DriverInclude.h"

/*
--------------------------------------------------------------------------------
  
                        Macro define  
  
--------------------------------------------------------------------------------
*/
#define min(x, y) (x < y ? x : y)
#define max(x, y) (x < y ? y : x)

//Emmc0 Config
#define EMMC_DEV0_RESET_IO   GPIOPortA_Pin0
#define EMMC_DEV0_BUS_WIDTH  BUS_WIDTH_8_BIT  
#define EMMC_DEV0_SDC_ID     0
#define EMMC_DEV0_DMA_ID     0

/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/




/*
--------------------------------------------------------------------------------
  
                        Variable Define
  
--------------------------------------------------------------------------------
*/
extern void * UartHDC;
extern uint32 SysTickCounter;
extern void (*pIdleFun)(void);
extern void (*pIdleFunSave)(void);


/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/
extern void BSP_Init(void);
extern void SysInterruptInit(void);
extern void GpioInit(void);
extern void SCUInit(void);
extern void poweron(void);
extern void CpuIdle(void);

extern void rk_printf(const char * buf, ...);
extern void rk_print_string(const uint8 * pstr);
extern void rk_printf_no_time(const char * fmt,...);
extern void rk_count_clk_start();
extern void rk_count_clk_end();

extern rk_err_t ShellCheckCmd(uint8 * CmdStr[],  uint8 * Cmd,  uint32 len);
extern uint32 ShellItemExtract(uint8 * pstr, uint8 ** pItem);

extern uint32 StrLenW(uint16 * pstr);
extern uint32 StrLenA(uint8 * pstr);
extern uint32 StringtoNum(uint8 * pstr);

extern int32 StrCmpW(uint16 * pstr1, uint16 * pstr2, uint32 len);
extern int32 StrCmpA(uint8 * pstr1, uint8 * pstr2, uint32 len);
extern rk_err_t CreateDevice(uint32 ClassID, uint32 DevID, void * arg);

#endif
