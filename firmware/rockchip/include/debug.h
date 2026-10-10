/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   typedef.h
*
* Description:  typedef.h -Macros define the data type
*
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

/*
*-------------------------------------------------------------------------------
*
* declare a external definition function for debug,it can call it when need.
*
*-------------------------------------------------------------------------------
*/
extern void __error__(char *pcFilename, unsigned long ulLine);

/*
*-------------------------------------------------------------------------------
*
* define DEGUG
*
*-------------------------------------------------------------------------------
*/

//#define DEBUG

#ifdef DEBUG
#define ASSERT(expr) {                                      \
                         if(!(expr))                        \
                         {                                  \
                             __error__(__FILE__, __LINE__); \
                         }                                  \
                     }
#else
#define ASSERT(expr)
#endif
/*
********************************************************************************
*
*                         End of Debug.h
*
********************************************************************************
*/
#endif
