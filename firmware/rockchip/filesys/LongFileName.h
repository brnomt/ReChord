/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   LongFileName.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _LONGFILENAME_H_
#define _LONGFILENAME_H_
#define LONG_FILENAME_FS
#undef  EXT
#ifdef  IN_LONG_FILE_NAME
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
#define DIRFLAG_DIR					(0X01)
#define DIRFLAG_FOUNDENTRY 			(0X01<<1)
#define DIRFLAG_MEETEMPTY			(0X01<<2) //had found,0x00 is mean a empty direction item.

/*
*-------------------------------------------------------------------------------
*  
*                          structure definition
*  
*-------------------------------------------------------------------------------
*/
typedef __packed struct _FDTLFN
{
	char		Name[11];		//short file name
	char		Attr;           //file property
	char		NTRes;          //reserved for NT
	char		CrtTimeTenth;	//build time（fat16 reservation）
    char	    reserve[18];    //reserved
} FDTLFN;


//long file name item
typedef __packed struct _FDTLDIR
{
	char		LDir_Ord;		 //short file name
	char		LDir_Name1[10];  //file property
	char		LDir_Attr;		 //reserved for NT
	char		LDir_Type;	     //build time（fat16 reservation）
	char		LDir_ChkSum;	 //build time（fat16 reservation）
	char		LDir_Name2[12];  //file property
	char		LDir_FstClu[2];  //last access date (fat16 reservation）
	char		LDir_Name3[4];	 //file property
} FDTLDIR;

typedef __packed struct _FdtEntryInfo
{
	uint16*		pUniLongName;
	char*		pShortName; //if duplicate name,need change.
	
	//for improving the speed of finding,external provide cache aeaa.
	uint16		*pOutTmpBuf;  // 
	uint16		OutTmpBufLen;  //the max only is 2k
	
	uint16		LongNameLen;
	
	uint16		DirEntryCount; //needed direntry number,short file name add long file name.
	
	uint8       DirFlag;   // bit 0: 1:dir,0 FIle, bit2: 1:find empty dir entry 
	uint8       NTSNFlag; // NT short file name flag.

	uint16		ReadOurLen;
	
	uint16		LNFoundCnt;  //the number of FDF that had been found can store the current direction name.
	uint8       LNLenMatch;	//whethe is match part or all with long file name,useed temporarily.
	uint16		LNSectOffet;//the offset of direction item,the range is a cluster.
	
	// = 0 mean do not find anything.
	// LN mean long file name,that is the FDF which has target
	uint32		LNFdtClus;	//current found CLUSTER it can store this direction entity.
	uint32		LNFdtLastClus;	//last cluster it be used in the situation of cross-cluster.
	uint16		LNFdtSectIndex; //the internal section offset of found cluster.

	//find the temporly variables that be used in process.
	uint16		FdtSectIndex;	//internal section offset of found cluster.
	uint32		FdtLBA; 	    //the LBA that is corresponding with current found CLUSTER;
	uint32		FdtClus;	//current found cluster.
	uint32		FdtLNIndex;  //the FDF offset of new build folder or folder is in father direction.
							 //it is used for new generated OBJHANDLE.
	
	uint8		FdtLNCheckSum;  //to record checksum that is used for filter the long incorrect file item.
}  FdtEntryInfo;

/*
--------------------------------------------------------------------------------
  
  Description:  变量定义
  
--------------------------------------------------------------------------------
*/


/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/

/*
********************************************************************************
*
*                         End of LongFileName.h
*
********************************************************************************
*/
#endif

