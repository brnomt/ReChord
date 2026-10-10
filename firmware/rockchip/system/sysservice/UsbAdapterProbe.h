/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   UsbAdapterProbe.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _USB_ADAPTER_H_
#define _USB_ADAPTER_H_

#undef  EXT
#ifdef _IN_USB_ADAPTER
#define EXT
#else
#define EXT extern
#endif

/*-----------------------------------------------------------------------------*/

extern void UsbAdpterProbeStart(void);
extern void UsbAdpterProbeStop(void);
extern int32 UsbAdpterProbe(void);
#ifdef _USB_HOST_
extern int32 USBHostEnumDevice (void);
#endif
/*
********************************************************************************
*
*                         End of UsbAdapterProbe.h
*
********************************************************************************
*/
#endif

