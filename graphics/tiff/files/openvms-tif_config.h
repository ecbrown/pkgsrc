#ifndef TIFF_OPENVMS_TIF_CONFIG_H
#define TIFF_OPENVMS_TIF_CONFIG_H

#include "tiffconf.h"

#define CCITT_SUPPORT 1
#define CHECK_JPEG_YCBCR_SUBSAMPLING 1

#define HAVE_ASSERT_H 1
#define HAVE_DECL_OPTARG 1
#define HAVE_FCNTL_H 1
#define HAVE_GETOPT 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1

/* OpenVMS x86-64 currently uses the 32-bit pointer ABI in pkgsrc. */
#define SIZEOF_SIZE_T 4
#define STRIP_SIZE_DEFAULT 8192
#define TIFF_MAX_DIR_COUNT 1048576
#define WORDS_BIGENDIAN 0

#define PACKAGE "tiff"
#define PACKAGE_NAME "LibTIFF Software"
#define PACKAGE_TARNAME "tiff"
#define PACKAGE_BUGREPORT "tiff@lists.osgeo.org"
#define PACKAGE_URL ""

#define TIFF_SIZE_FORMAT "zu"
#define TIFF_SSIZE_FORMAT PRId32

#endif
