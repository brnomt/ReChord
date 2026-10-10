/*
********************************************************************************************
*
*        Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Common\Driver\BB\BBSystem.h
* Owner: aaron.sun
* Date: 2015.6.5
* Time: 10:18:57
* Version: 1.0
* Desc: BB system drvier
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.5     10:18:57   1.0
********************************************************************************************
*/


#ifndef __COMMON_DRIVER_BB_BBSYSTEM_H__
#define __COMMON_DRIVER_BB_BBSYSTEM_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define                                     
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define                         
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _COMMON_DRIVER_BB_BBSYSTEM_COMMON_  
#define _COMMON_DRIVER_BB_BBSYSTEM_INIT_  
#define _COMMON_DRIVER_BB_BBSYSTEM_SHELL_  
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare                             
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum _MSGBOX_SYSTEM_CMD
{
    MSGBOX_CMD_SYSTEM_NULL,   

    //for BB system control
    MSGBOX_CMD_SYSTEM_START_OK,
    MSGBOX_CMD_BB_HOLD,
    MSGBOX_CMD_BB_HOLD_ACK,
    MSGBOX_CMD_BB_HOLD_EXIT,

    //for BB system debug
    MSGBOX_CMD_SYSTEM_PRINT_LOG,
    MSGBOX_CMD_SYSTEM_PRINT_LOG_OK,
    
    MSGBOX_CMD_SYSTEM_NUM
    
}MSGBOX_SYSTEM_CMD;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare                                       
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void BBSystemDeInit(void);
extern void ShutOffBBSystem(void);
extern void BBSystemInit(void);
extern int32 StartBBSystem(uint32 ModuleId);



#endif
