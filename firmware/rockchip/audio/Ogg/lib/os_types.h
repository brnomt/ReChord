/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2002    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: #ifdef jail to whip a few platforms into the UNIX ideal.

 ********************************************************************/
#ifndef __INT64_DEF
#define __INT64_DEF
typedef long long __int64;
#endif
#ifndef _OS_TYPES_H
#define _OS_TYPES_H

#define _LOW_ACCURACY_

#  define X(n) (((((n)>>22)+1)>>1) - ((((n)>>22)+1)>>9))
#  define LOOKUP_T unsigned char

/* make it easy on the folks that want to compile the libs with a
   different malloc than stdlib */
#define _ogg_malloc		rm_memory_default_malloc
#define _ogg_free		rm_memory_default_free

#define _ogg_calloc  RK_ogg_calloc
#define _ogg_realloc RK_ogg_realloc
//#define _ogg_malloc  malloc
//#define _ogg_free    free

//#define RK_ogg_alloca test_malloc

   /* MSVC/Borland */
   /*
   typedef __int64 ogg_int64_t;
   typedef __int32 ogg_int32_t;
   typedef unsigned __int32 ogg_uint32_t;
   typedef __int16 ogg_int16_t;
   typedef unsigned __int16 ogg_uint16_t;
   */
   /* OS/2 GCC */
   typedef short ogg_int16_t;
   typedef unsigned short ogg_uint16_t;
   typedef int ogg_int32_t;
   typedef unsigned int ogg_uint32_t;
   typedef __int64 ogg_int64_t;


#endif  /* _OS_TYPES_H */
