/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  BroCore.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             chenfen          2008-3-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_BRO_CORE_

#include "SysInclude.h"

#ifdef  _BROWSER_

#include "fsinclude.h"
#include "BrowserUI.h"
#include "BroCore.h"

/*
--------------------------------------------------------------------------------
  Function name : void BroFileDelete(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_CORE_CODE_
uint8 BroFileDelete(uint32 DirClus, uint8 *DirFileName)
{
	int32 ClusIndex1;
	int8 FileName[12];
	int8 Rt;
	FDT temp;

    Rt = NOT_FIND_FILE;

    #ifdef ENCODE
    memcpy(FileName,DirFileName,11);

    if (RETURN_OK == FindFDTInfo(&temp, DirClus, FileName))		//find direction item
    {
        if ((temp.Attr & ATTR_DIRECTORY) == 0)  		//only file can delete.
        {
            Rt = FILE_LOCK;
            if (FindOpenFile(DirClus, DirFileName) >= MAX_OPEN_FILES)	//if file have opened,it cannot be deleted
            {
                FLASH_PROTECT_OFF();
                ClusIndex1 = temp.FstClusLO + ((uint32)(temp.FstClusHI) << 16);
                FATDelClusChain(ClusIndex1);
                Rt = DelFDT(DirClus, FileName);
                Rt = RETURN_OK;//2008-2-2 zyf
                FLASH_PROTECT_ON() ;
            }
        }
    }
    #endif

    return (Rt);
}


/*
--------------------------------------------------------------------------------
  Function name : BroGetFileInfo
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_CORE_CODE_
BroFileInfo * BroGetFileInfo(uint8 * FileName)
{
	BroFileInfo * FileInfo = gBroFileInfo;

	while( FileInfo->FileType!= FileTypeALL)
	{
		if(TRUE == FileExtNameMatch(FileName+8,FileInfo->FileExtName, ATTR_ARCHIVE))
		{
			return FileInfo;
		}
		FileInfo++;
	}
	return FileInfo;
}

/*
--------------------------------------------------------------------------------
  Function name : GetSearchFileInfo
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_CORE_CODE_
BroFileInfo *GetSearchFileInfo(FileType FileType)
{
	BroFileInfo * FileInfo = gBroFileInfo;

	while( FileInfo->FileType!= FileTypeALL)
	{
		if(FileInfo->FileType == FileType)
		{
		    break;
		}
		FileInfo++;
	}
	return (FileInfo);
}

/*
--------------------------------------------------------------------------------
  Function name : BroCheckFileType
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_CORE_CODE_
uint16 BroCheckFileType( uint8 *SrcExtName, FileType FileType )
{
	BroFileInfo * FileInfo;

	if( FileType == FileTypeALL)
	{
		return TRUE;
	}

    FileInfo = BroGetFileInfo( SrcExtName );

	return( FileInfo->FileType == FileType );
}
/*
********************************************************************************
*
*                         End of BroCore.c
*
********************************************************************************
*/
#endif
