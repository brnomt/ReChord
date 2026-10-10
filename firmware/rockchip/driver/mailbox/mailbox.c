/*
********************************************************************************************
*
*                Copyright (c): 2014 - 2014 + 5, aaron.sun
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\mailbox.c
* Owner: aaron.sun
* Date: 2014.11.19
* Time: 8:47:47
* Desc: mailbox for 2 m3
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2014.11.19     8:47:47   1.0
********************************************************************************************
*/

#include "SysInclude.h"

#if 1//def __CPU_NANOC_LIB_MAILBOX_C__
#include "hw_memap.h"
#include "hw_mailbox.h"
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

#define RK_SUCCESS           RETURN_OK

#define _CPU_NANOC_LIB_MAILBOX_READ_  
#define _CPU_NANOC_LIB_MAILBOX_WRITE_ 
#define _CPU_NANOC_LIB_MAILBOX_INIT_  
#define _CPU_NANOC_LIB_MAILBOX_SHELL_  


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MailBoxClearA2BInt
** Input:uint32 MailBoxID, uint32 IntType
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 15:41:21
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_READ_
READ API rk_err_t MailBoxClearA2BInt(uint32 MailBoxID, uint32 IntType)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    pMailBox->MailBoxA2BStatus = IntType;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MailBoxClearB2AInt
** Input:uint32 MailBoxID, uint32 IntType
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 15:39:17
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_READ_
READ API rk_err_t MailBoxClearB2AInt(uint32 MailBoxID, uint32 IntType)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    pMailBox->MailBoxB2AStatus = IntType;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MailBoxReadB2ACmd
** Input:uint32 MailBoxID, uint32 CmdPort
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 9:03:24
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_READ_
READ API uint32 MailBoxReadB2ACmd(uint32 MailBoxID, uint32 CmdPort)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    uint32 Cmd;

    switch(CmdPort)
    {
        case 0:
            Cmd = pMailBox->MailBoxB2ACmd0;
            break;
        case 1:
            Cmd = pMailBox->MailBoxB2ACmd1;
            break;
        case 2:
            Cmd = pMailBox->MailBoxB2ACmd2;
            break;
        case 3:
            Cmd = pMailBox->MailBoxB2ACmd3;
            break;
        default:
            Cmd = 0;
            break;
    }

    return Cmd;
}

/*******************************************************************************
** Name: MailBoxReadB2AData
** Input:uint32 MailBoxID, uint32 DataPort
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 9:02:43
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_READ_
READ API uint32 MailBoxReadB2AData(uint32 MailBoxID, uint32 DataPort)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);

    uint32 Data;

    switch(DataPort)
    {
        case 0:
            Data = pMailBox->MailBoxB2AData0;
            break;
        case 1:
            Data = pMailBox->MailBoxB2AData1;
            break;
        case 2:
            Data = pMailBox->MailBoxB2AData2;
            break;
        case 3:
            Data = pMailBox->MailBoxB2AData3;
            break;
        default:
            Data = 0;
            break;
    }

    return Data;
}

/*******************************************************************************
** Name: MailBoxReadA2BData
** Input:uint32 MailBoxID, uint32 DataPort
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 8:52:32
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_READ_
READ API uint32 MailBoxReadA2BData(uint32 MailBoxID, uint32 DataPort)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);

    uint32 Data;

    switch(DataPort)
    {
        case 0:
            Data = pMailBox->MailBoxA2BData0;
            break;
        case 1:
            Data = pMailBox->MailBoxA2BData1;
            break;
        case 2:
            Data = pMailBox->MailBoxA2BData2;
            break;
        case 3:
            Data = pMailBox->MailBoxA2BData3;
            break;
        default:
            Data = 0;
            break;
    }

    return Data;
}

/*******************************************************************************
** Name: MailBoxReadA2BCmd
** Input:uint32 MailBoxID, uint32 CmdPort
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 8:48:24
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_READ_
READ API uint32 MailBoxReadA2BCmd(uint32 MailBoxID, uint32 CmdPort)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    uint32 Cmd;

    switch(CmdPort)
    {
        case 0:
            Cmd = pMailBox->MailBoxA2BCmd0;
            break;
        case 1:
            Cmd = pMailBox->MailBoxA2BCmd1;
            break;
        case 2:
            Cmd = pMailBox->MailBoxA2BCmd2;
            break;
        case 3:
            Cmd = pMailBox->MailBoxA2BCmd3;
            break;
        default:
            Cmd = 0;
            break;
    }

    return Cmd;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MailBoxWriteB2ACmd
** Input:uint32 Cmd, uint32 MailBoxID, uint32 CmdPort
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 9:02:04
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_WRITE_
WRITE API rk_err_t MailBoxWriteB2ACmd(uint32 Cmd, uint32 MailBoxID, uint32 CmdPort)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);

    switch(CmdPort)
    {
        case 0:
            pMailBox->MailBoxB2ACmd0 = Cmd;
            break;
        case 1:
            pMailBox->MailBoxB2ACmd1 = Cmd;
            break;
        case 2:
            pMailBox->MailBoxB2ACmd2 = Cmd;
            break;
        case 3:
            pMailBox->MailBoxB2ACmd3 = Cmd;
            break;
        default:
            break;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MailBoxWriteB2AData
** Input:uint32 Data, uint32 MailBoxID, uint32 DataPort
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 8:58:23
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_WRITE_
WRITE API rk_err_t MailBoxWriteB2AData(uint32 Data, uint32 MailBoxID, uint32 DataPort)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);

    switch(DataPort)
    {
        case 0:
            pMailBox->MailBoxB2AData0 = Data;
            break;
        case 1:
            pMailBox->MailBoxB2AData1 = Data;
            break;
        case 2:
            pMailBox->MailBoxB2AData2 = Data;
            break;
        case 3:
            pMailBox->MailBoxB2AData3 = Data;
            break;
        default:
            break;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MailBoxWriteA2BData
** Input:uint32 Data, uint32 MailBoxID, uint32 DataPort
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 8:53:27
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_WRITE_
WRITE API rk_err_t MailBoxWriteA2BData(uint32 Data, uint32 MailBoxID, uint32 DataPort)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    switch(DataPort)
    {
        case 0:
            pMailBox->MailBoxA2BData0 = Data;
            break;
        case 1:
            pMailBox->MailBoxA2BData1 = Data;
            break;
        case 2:
            pMailBox->MailBoxA2BData2 = Data;
            break;
        case 3:
            pMailBox->MailBoxA2BData3 = Data;
            break;
        default:
            break;
    }

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: MailBoxWriteA2BCmd
** Input:uint32 Cmd, uint32 MailBoxID, uint32 CmdPort
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 8:51:17
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_WRITE_
WRITE API rk_err_t MailBoxWriteA2BCmd(uint32 Cmd, uint32 MailBoxID, uint32 CmdPort)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);

    switch(CmdPort)
    {
        case 0:
            pMailBox->MailBoxA2BCmd0 = Cmd;
            break;
        case 1:
            pMailBox->MailBoxA2BCmd1 = Cmd;
            break;
        case 2:
            pMailBox->MailBoxA2BCmd2 = Cmd;
            break;
        case 3:
            pMailBox->MailBoxA2BCmd3 = Cmd;
            break;
        default:
            break;
    }

    return RK_SUCCESS;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MailBoxDisableA2BInt
** Input:uint32 MailBoxID, uint32 Port
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 11:49:19
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_INIT_
INIT API rk_err_t MailBoxDisableA2BInt(uint32 MailBoxID, uint32 IntType)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    pMailBox->MailBoxA2BIntEn &= ~IntType;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MailBoxDisableB2AInt
** Input:uint32 MailBoxID, uint32 Port
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 11:48:23
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_INIT_
INIT API rk_err_t MailBoxDisableB2AInt(uint32 MailBoxID, uint32 IntType)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    pMailBox->MailBoxB2AIntEn &= ~IntType;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MailBoxEnalbeB2AInt
** Input:uint32 MailBoxID, uint32 Port
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 11:42:55
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_INIT_
INIT API rk_err_t MailBoxEnableB2AInt(uint32 MailBoxID, uint32 IntType)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    pMailBox->MailBoxB2AIntEn |= IntType;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MailBoxEnableA2BInt
** Input:uint32 MailBoxID, uint32 Port
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.19
** Time: 11:36:13
*******************************************************************************/
_CPU_NANOC_LIB_MAILBOX_INIT_
INIT API rk_err_t MailBoxEnableA2BInt(uint32 MailBoxID, uint32 IntType)
{
    MAIL_BOX * pMailBox = MailBoxPort(MailBoxID);
    pMailBox->MailBoxA2BIntEn |= IntType;
    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

