/* Exercise the OpenVMS static library through both write and RGBA read paths. */
#include "tiffio.h"

#include <stdio.h>

int
main(void)
{
    static const unsigned char rows[2][6] = {
        {255, 0, 0, 0, 255, 0},
        {0, 0, 255, 255, 255, 255}
    };
    const char *name = "openvms-test.tif";
    uint32_t raster[4];
    TIFF *tif;
    int row;

    tif = TIFFOpen(name, "w");
    if (tif == NULL)
        return 1;
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 2);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 2);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_ADOBE_DEFLATE);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 2);
    for (row = 0; row < 2; ++row)
    {
        if (TIFFWriteScanline(tif, (void *)rows[row], (uint32_t)row, 0) < 0)
        {
            TIFFClose(tif);
            return 2;
        }
    }
    TIFFClose(tif);

    tif = TIFFOpen(name, "r");
    if (tif == NULL)
        return 3;
    if (!TIFFReadRGBAImageOriented(tif, 2, 2, raster,
                                   ORIENTATION_TOPLEFT, 0))
    {
        TIFFClose(tif);
        return 4;
    }
    TIFFClose(tif);

    if (TIFFGetR(raster[0]) < 240 || TIFFGetG(raster[0]) > 15 ||
        TIFFGetB(raster[0]) > 15)
        return 5;
    if (TIFFGetG(raster[1]) < 240)
        return 6;
    if (TIFFGetB(raster[2]) < 240)
        return 7;

    (void)remove(name);
    puts("OpenVMS libtiff write/read test passed");
    return 0;
}
