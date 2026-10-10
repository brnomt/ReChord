/************************************************************************
 *       Smart Jpeg Decoder
 *
 * File:
 *  parser.c
 *
************************************************************************/
#include "image_main.h"

#ifdef JPG_DEC_INCLUDE

#pragma arm section code = "JpgDecCode", rodata = "JpgDecCode", rwdata = "JpgDecData", zidata = "JpgDecBss"
#include "jpgdec_global.h"
#include "jpgdec_decompress.h"
#include "jpgdec_globalvardeclare.h"
// Read a quantization table.
int JpgDecReadDqtMarker(void)
{
    int32  n, i, prec;
    uint32 left;
    uint32 temp;

    left = JpgDecGetBits_1(16);

    if (left < 2)
    {
        //return(JPGD_BAD_DQT_MARKER);
        return JPGD_BAD_DQT_MARKER;
    }

    left -= 2;

    while (left)
    {
        n = JpgDecGetBits_1(8);
        prec = n >> 4;
        n &= 0x0F;

        if (n >= JPGD_MAXQUANTTABLES)
        {
            //return(JPGD_BAD_DQT_TABLE);
            return JPGD_BAD_DQT_TABLE;
        }

//        if (!Jpg_pQuant[n])
            //Jpg_pQuant[n] = (QUANT_TYPE *)JpgDecAlloc(64 * sizeof(QUANT_TYPE));
//            Jpg_pQuant[n] = gJpg_pQuant[n];

        if (Jpg_pQuant[n] == NULL)
            return JPGD_TOO_MANY_BLOCKS;
        // read quantization entries, in zag order
        for (i = 0; i < 64; i++)
        {
            temp = JpgDecGetBits_1(8);

            if (prec)
                temp = (temp << 8) + JpgDecGetBits_1(8);

            Jpg_pQuant[n][i] = (int16) temp;
        }

        i = 64 + 1;

        if (prec)
            i += 64;

        if (left < (uint32)i)
        {
            //return(JPGD_BAD_DQT_LENGTH);
            return JPGD_BAD_DQT_LENGTH;
        }

        left -= i;
    }
    return 0;
}

// Read a Huffman code table.
int JpgDecReadDhtMarker(void)
{
    int32 i, index, count;
    uint32 left;
    uint8 huff_number[17];
    uint8 huff_value[256];

    left = JpgDecGetBits_1(16);

    if (left < 2)
    {
        //return(JPGD_BAD_DHT_MARKER);
        return JPGD_BAD_DHT_MARKER;
    }

    left -= 2;

    while (left)
    {
        index = JpgDecGetBits_1(8);

        huff_number[0] = 0;

        count = 0;

        for (i = 1; i <= 16; i++)
        {
            huff_number[i] = JpgDecGetBits_1(8);
            count += huff_number[i];
        }

        if (count > 255)
        {
            //return(JPGD_BAD_DHT_COUNTS);
            return JPGD_BAD_DHT_COUNTS;
        }

        for (i = 0; i < count; i++)
            huff_value[i] = JpgDecGetBits_1(8);

        i = 1 + 16 + count;

        if (left < (uint32)i)
        {
            //return(JPGD_BAD_DHT_MARKER);
            return JPGD_BAD_DHT_MARKER;
        }

        left -= i;

        if ((index & 0x10) > 0x10)
        {
            //return(JPGD_BAD_DHT_INDEX);
            return JPGD_BAD_DHT_INDEX;
        }

        index = (index & 0x0F) + ((index & 0x10) >> 4) * (JPGD_MAXHUFFTABLES >> 1);

        if (index >= JPGD_MAXHUFFTABLES)
        {
            return(JPGD_BAD_DHT_INDEX);
        }

 //       if (!Jpg_pHuffNum[index])
            //Jpg_pHuffNum[index] = (uint8 *)JpgDecAlloc(17);
//            Jpg_pHuffNum[index] = gJpg_pHuffNum[index];

        if (Jpg_pHuffNum[index] == NULL)
            return JPGD_TOO_MANY_BLOCKS;

//        if (!Jpg_pHuffVal[index])
//            Jpg_pHuffVal[index] = gJpg_pHuffVal[index];//(uint8 *)JpgDecAlloc(256);

        if (Jpg_pHuffVal[index] == NULL)
            return JPGD_TOO_MANY_BLOCKS;

        memcpy(Jpg_pHuffNum[index], huff_number, 17);
        memcpy(Jpg_pHuffVal[index], huff_value, 256);
    }
    return 0;
}

// Read the start of frame (SOF) marker.
int JpgDecReadSofMarker(void)
{
    int i;
    uint left;

    left = JpgDecGetBits_1(16);

    if (JpgDecGetBits_1(8) != 8)   /* precision: sorry, only 8-bit precision is supported right now */
    {
        //return(JPGD_BAD_PRECISION);
        return JPGD_BAD_PRECISION;
    }

    Jpg_gImageYSize = JpgDecGetBits_1(16);

    if ((Jpg_gImageYSize < 1) || (Jpg_gImageYSize > JPGD_MAX_HEIGHT))
    {
        //return(JPGD_BAD_HEIGHT);
        return JPGD_BAD_HEIGHT;
    }

    Jpg_gImageXSize = JpgDecGetBits_1(16);

    if ((Jpg_gImageXSize < 1) || (Jpg_gImageXSize > JPGD_MAX_WIDTH))
    {
        //return(JPGD_BAD_WIDTH);
        return JPGD_BAD_HEIGHT;
    }

    Jpg_gComponentsInFrame = JpgDecGetBits_1(8);

    if (Jpg_gComponentsInFrame > JPGD_MAXCOMPONENTS)
    {
        //return(JPGD_TOO_MANY_COMPONENTS);
        return JPGD_TOO_MANY_COMPONENTS;
    }

    if (left != (uint)(Jpg_gComponentsInFrame * 3 + 8))
    {
        //return(JPGD_BAD_SOF_LENGTH);
        return JPGD_BAD_SOF_LENGTH;
    }

    for (i = 0; i < Jpg_gComponentsInFrame; i++)
    {
        Jpg_gCompIdent[i]  = JpgDecGetBits_1(8);
        Jpg_gCompHoriSamp[i] = JpgDecGetBits_1(4);
        Jpg_gCompVertSamp[i] = JpgDecGetBits_1(4);
        Jpg_gCompQuant[i]  = JpgDecGetBits_1(8);
    }
    return 0;
}

// Read a define restart interval (DRI) marker.
int JpgDecReadDriMarker(void)
{
    if (JpgDecGetBits_1(16) != 4)
    {
        //return(JPGD_BAD_DRI_LENGTH);
        return JPGD_BAD_DRI_LENGTH;
    }

    Jpg_gRestartInterval = JpgDecGetBits_1(16);
    return 0;
}

// Finds the next marker.
int JpgDecNextMarker(void)
{
    uint c, bytes;

    bytes = 0;

    do
    {
        do
        {
            bytes++;

            c = JpgDecGetBits_1(8);

        }
        while (c != 0xFF);

        do
        {
            c = JpgDecGetBits_1(8);

        }
        while (c == 0xFF);

    }
    while (c == 0);

    // If bytes > 0 here, there where extra bytes before the marker (not good).

    return c;
}
//------------------------------------------------------------------------------
// Process markers. Returns when an SOFx, SOI, EOI, or SOS marker is
// encountered.
int JpgDecProcessMarkers(void)
{
    int c;

    for (; ;)
    {
        c = JpgDecNextMarker();

        switch (c)
        {
            case M_SOF0:
            case M_SOF1:
            case M_SOF2:
            case M_SOF3:
            case M_SOF5:
            case M_SOF6:
            case M_SOF7:
//      case M_JPG:
            case M_SOF9:
            case M_SOF10:
            case M_SOF11:
            case M_SOF13:
            case M_SOF14:
            case M_SOF15:
            case M_SOI:
            case M_EOI:
            case M_SOS:
                {
                    return c;
                }
            case M_DHT:
                {
                    c = JpgDecReadDhtMarker();
                    if (c)
                        return c;

                    break;
                }
                // Sorry, no arithmitic support at this time. Dumb patents!
            case M_DAC:
                {
                    //return(JPGD_NO_ARITHMITIC_SUPPORT);
                    break;
                }
            case M_DQT:
                {
                    c = JpgDecReadDqtMarker();
                    if (c)
                        return c;
                    break;
                }
            case M_DRI:
                {
                    c = JpgDecReadDriMarker();
                    if (c)
                        return c;
                    break;
                }
                //case M_APP0:  /* no need to read the JFIF marker */

            case M_JPG:
            case M_RST0:    /* no parameters */
            case M_RST1:
            case M_RST2:
            case M_RST3:
            case M_RST4:
            case M_RST5:
            case M_RST6:
            case M_RST7:
            case M_TEM:
                {
                    //return(JPGD_UNEXPECTED_MARKER);
                    break;
                }
            default:    /* must be DNL, DHP, EXP, APPn, JPGn, COM, or RESn or APP0 */
                {
                    JpgDecSkipVariableMarker();
                    break;
                }

        }
    }
}
//------------------------------------------------------------------------------
// Finds the start of image (SOI) marker.
// This code is rather defensive: it only checks the first 512 bytes to avoid
// false positives.
int JpgDecLocateSoiMarker(void)
{
    uint lastchar, thischar;
    ulong bytesleft;

    lastchar = JpgDecGetBits_1(8);

    thischar = JpgDecGetBits_1(8);

    /* ok if it's a normal JPEG file without a special header */

    if ((lastchar == 0xFF) && (thischar == M_SOI))
        return 0;

    bytesleft = 512;

    for (; ;)
    {
        if (--bytesleft == 0)
        {
            return(JPGD_NOT_JPEG);
        }

        lastchar = thischar;

        thischar = JpgDecGetBits_1(8);

        if ((lastchar == 0xFF) && (thischar == M_SOI))
            break;
    }

    /* Check the next character after marker: if it's not 0xFF, it can't
         be the start of the next marker, so it probably isn't a JPEG */

    thischar = (Jpg_gBitBuf >> 8) & 0xFF;

    if (thischar != 0xFF)
    {
        return(JPGD_NOT_JPEG);
    }
    return 0;
}

//------------------------------------------------------------------------------
// Find a start of frame (SOF) marker.
int JpgDecLocateSofMarker(void)
{
    int c;

    c = JpgDecLocateSoiMarker();
    if (c)
        return c;

    c = JpgDecProcessMarkers();

    switch (c)
    {
        case M_SOF2:  // progressive DCT
            {
				return JPG_PROGRESSIVE;
            }
        case M_SOF0:  /* baseline DCT */
        case M_SOF1:  /* extended sequential DCT */
            //case M_SOF2:
            {
                c = JpgDecReadSofMarker();
                if (c)
                    return c;

                break;
            }
        case M_SOF9:  /* Arithmitic coding */
            {
                break;
            }

        default:
            {
                break;
            }
    }
	return 0;
}

//------------------------------------------------------------------------------
// Find a start of scan (SOS) marker.
int JpgDecLocateSosMarker(void)
{
    int c;

    c = JpgDecProcessMarkers();

    if (c == M_EOI)
        return FALSE;
    else if (c != M_SOS)
    {
        //return(JPGD_UNEXPECTED_MARKER);
        //exit(1);
        return FALSE;
    }
    c = JpgDecReadSosMarker();
    if (c)
        return FALSE;

    return TRUE;
}
// Read a start of scan (SOS) marker.
int  JpgDecReadSosMarker(void)
{
    uint32 left;
    int32 i, ci, n, c, cc;

    left = JpgDecGetBits_1(16);

    n = JpgDecGetBits_1(8);

    Jpg_gCompInScan = n;

    left -= 3;

    if ((left != (uint)(n * 2 + 3)) || (n < 1) || (n > JPGD_MAXCOMPSINSCAN))
    {
        //return(JPGD_BAD_SOS_LENGTH);
        return JPGD_BAD_SOS_LENGTH;
    }

    for (i = 0; i < n; i++)
    {
        cc = JpgDecGetBits_1(8);
        c = JpgDecGetBits_1(8);
        left -= 2;

        for (ci = 0; ci < Jpg_gComponentsInFrame; ci++)
            if (cc == Jpg_gCompIdent[ci])
                break;

        if (ci >= Jpg_gComponentsInFrame)
        {
            //return(JPGD_BAD_SOS_COMP_ID);
            return JPGD_BAD_SOS_COMP_ID;
        }

        Jpg_gCompList[i]    = ci;
        Jpg_gCompDCTable[ci] = (c >> 4) & 15;
        Jpg_gCompACTable[ci] = (c & 15) + (JPGD_MAXHUFFTABLES >> 1);
    }

    Jpg_gSpectralStart  = JpgDecGetBits_1(8);
    Jpg_gSpectralEnd    = JpgDecGetBits_1(8);
    Jpg_gSuccessiveHigh = JpgDecGetBits_1(4);
    Jpg_gSuccessiveLow  = JpgDecGetBits_1(4);

    //if (!progressive_flag)
    {

        Jpg_gSpectralStart = 0;
        Jpg_gSpectralEnd = 63;
    }


    left -= 3;

    while (left)                  /* read past whatever is left */
    {
        JpgDecGetBits_1(8);
        left--;
    }
    return 0;
}

// Used to skip unrecognized markers.
int JpgDecSkipVariableMarker(void)
{
    uint left;

    left = JpgDecGetBits_1(16);

    if (left < 2)
    {
        return(JPGD_BAD_VARIABLE_MARKER);
    }

    left -= 2;

    while (left)
    {
        JpgDecSkipBits_1(8);
        left--;
    }
    return 0;
}

// Find end of image (EOI) marker, so we can return to the user the
// exact size of the input stream.
void JpgDecFindEOI(void)
{

    // Attempt to read the EOI marker.
    //JpgDecGetBits_2(Jpg_gBitsLeft & 7);

    // Prime the bit buffer
    Jpg_gBitsLeft = 16;
    //Jpg_gBitBuf = 0;
    JpgDecGetBits_1(16);
    JpgDecGetBits_1(16);

    // The next marker _should_ be EOI
    JpgDecProcessMarkers();

// Jpg_gTotalBytesRead -= Jpg_gInBufLeft;
}
#pragma arm section code
#endif

