#include "Driverconfig.h"
#include "../ImageInclude/image_main.h"
#include "../ImageInclude/image_globals.h"
#include "ImageControl.h"
#ifdef _PICTURE_
#pragma arm section code = "ImageContrlCode", rodata = "ImageContrlCode", rwdata = "ImageContrlData", zidata = "ImageContrlBss"
int get_output_wh(int *w,int *h,int image_w,int image_h)
{
#ifdef AUTO_ADAPT_TO_LCD
    int long_side,short_side;
    long_side = IMAGE_MAX(ImageMaxWidth,ImageMaxHeight);
    short_side = IMAGE_MIN(ImageMaxWidth,ImageMaxHeight);
//    if ((ImageMaxWidth >= image_w)&&(ImageMaxHeight >= image_h))
//    {
//        *w = ImageMaxWidth;
//        *h = ImageMaxHeight;
//        return IMAGE_HORIZONTLE;
//    }

    if (image_w >= image_h)
    {
        *w = long_side;
        *h = short_side;
    }
    else
    {
        *h = long_side;
        *w = short_side;
    }


    if (*w == ImageMaxWidth)
        return IMAGE_HORIZONTLE;
    else
        return IMAGE_VERTIAL;
#else
    *w = ImageMaxWidth;
    *h = ImageMaxHeight;
    return IMAGE_HORIZONTLE;
#endif
}
#pragma arm section code
#endif
