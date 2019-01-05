//***********************************************************************************************
//    Program      : Tag Image File Format Read/Write Header                    
//    File          : TiffIO.h                                                    
//    Function     : write_tiff_image, read_tiff_image,desample    
//  Last up data : 2003. 1. 12 by Byun Seung Chan(byun2133@sogang.ac.kr)
//    Revision    : 01/25/99    by Jeong-Suk, Jason, Yoon
//************************************************************************************************

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "tiff.h"
#include "tiffio.h"

int write_tiff_image(char *filename, unsigned char *in_image, int height, int width, int mode)
{
    TIFF            *out;

    unsigned char   rmap[256],gmap[256],bmap[256];
    int              i;
    int                color_set;
    
    out = TIFFOpen(filename, "w");
    if (!out)
    {
        fprintf(stderr, "Can't open %s for TIFF output\n", filename);
        return (-1);
    }

    if (mode==0)
    {
        color_set = 1;
        TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );
    }
    else
    {
        color_set = 3;
        TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
    }
    
    for(i=0; i<256; i++)
    {
        rmap[i] = i;
        gmap[i] = i;
        bmap[i] = i;
    }

    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, color_set);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(out, TIFFTAG_COLORMAP, rmap, gmap, bmap);
    TIFFWriteEncodedStrip(out,0, in_image, width*height*color_set);
    TIFFClose(out);
    return (0);
}

int read_tiff_image(char *filename, unsigned char **image, int *height, int *width, int mode)
{
    TIFF                   *in;
    unsigned short int        bitspersample, photometric;
    unsigned long int         h, w;
    int                     bit;
    unsigned char              *inbuf, *tbuf, *timage;
    register unsigned long int     row, column;
    int                        color_set;

    in = TIFFOpen(filename, "r");
    
    if (in == NULL)
    {
    return (-1);
    }
    if (mode == 0)
    {
        color_set = 1;
    }
    else
    {
        color_set = 3;
    }

    TIFFGetField(in, TIFFTAG_BITSPERSAMPLE, &bitspersample);
    if (bitspersample == 8)
    {
        TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w);
        *width = (int) w;
        TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h);
        *height = (int) h;
        TIFFGetField(in, TIFFTAG_PHOTOMETRIC, &photometric);

        if (((inbuf = (unsigned char *) malloc(TIFFScanlineSize(in) + 1)) == NULL) || ((*image = (unsigned char *) malloc(w * h * color_set *sizeof(unsigned char))) == NULL))
        {
            return (-1);
        }

        for (timage = *image, row = 0; row < h; row++)
        {
            TIFFReadScanline(in, inbuf, row, 0);
            tbuf = inbuf;
            for (column = 0; column < w*color_set; column++)
            {
                 if (photometric == PHOTOMETRIC_MINISWHITE)
                    *tbuf = ~(*tbuf);
                *timage++ = *tbuf ;
                *tbuf++;
            }
        }
    }
    else if(bitspersample == 1) 
    {
        TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w);
        *width = (int) w;
        TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h);
        *height = (int) h;
        TIFFGetField(in, TIFFTAG_PHOTOMETRIC, &photometric);


        if (((inbuf = (unsigned char *) malloc(TIFFScanlineSize(in) + 1)) == NULL) ||
            ((*image = (unsigned char *) malloc(w * h * sizeof(unsigned char))) == NULL))
            return (-1);

        for (timage = *image, row = 0; row < h; row++)
        {
            TIFFReadScanline(in, inbuf, row, 0);
            tbuf = inbuf;
            if (photometric == PHOTOMETRIC_MINISWHITE)
            *tbuf = ~(*tbuf);
            bit = 0;
            for (column = 0; column < w; column++)
            {
                *timage++ = ((*tbuf & (0200) >> bit) != 0) ? 255 : 0;
                if (++bit == 8)
                {
                    bit = 0;
                    *tbuf++;
                    if (photometric == PHOTOMETRIC_MINISWHITE)
                    *tbuf = ~(*tbuf);
                }
            }
        }
    }
    else 
    {
        return (-1);
    }
    TIFFClose(in);
    free(inbuf);
    return (0);
}
