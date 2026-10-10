/*
********************************************************************************************
*
*				  Copyright (c): 2014 - 2014 + 5, aaron.sun
*							   All rights reserved.
*
* FileName: Cpu\NanoC\lib\mailbox.h
* Owner: aaron.sun
* Date: 2014.11.19
* Time: 8:50:27
* Desc: 
* History:
*    <author>	 <date> 	  <time>	 <version>	   <Desc>
*    aaron.sun     2014.11.19     8:50:27   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_MAILBOX_H__
#define __CPU_NANOC_LIB_MAILBOX_H__

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
#define MAILBOX_INT_0 (uint32)(1 << 0)
#define MAILBOX_INT_1 (uint32)(1 << 1)
#define MAILBOX_INT_2 (uint32)(1 << 2)
#define MAILBOX_INT_3 (uint32)(1 << 3)

#define MAILBOX_CHANNEL_0 (0)
#define MAILBOX_CHANNEL_1 (1)
#define MAILBOX_CHANNEL_2 (2)
#define MAILBOX_CHANNEL_3 (3)

#define MAILBOX_ID_0 (0) 
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare							 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare          							 
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t MailBoxClearA2BInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxClearB2AInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxDisableA2BInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxDisableB2AInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxEnableB2AInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxEnableA2BInt(uint32 MailBoxID, uint32 IntType);
extern uint32 MailBoxReadB2ACmd(uint32 MailBoxID, uint32 CmdPort);
extern uint32 MailBoxReadB2AData(uint32 MailBoxID, uint32 DataPort);
extern rk_err_t MailBoxWriteB2ACmd(uint32 Cmd, uint32 MailBoxID, uint32 CmdPort);
extern rk_err_t MailBoxWriteB2AData(uint32 Data, uint32 MailBoxID, uint32 DataPort);
extern rk_err_t MailBoxWriteA2BData(uint32 Data, uint32 MailBoxID, uint32 DataPort);
extern uint32 MailBoxReadA2BData(uint32 MailBoxID, uint32 DataPort);
extern rk_err_t MailBoxWriteA2BCmd(uint32 Cmd, uint32 MailBoxID, uint32 CmdPort);
extern uint32 MailBoxReadA2BCmd(uint32 MailBoxID, uint32 CmdPort);


#endif

