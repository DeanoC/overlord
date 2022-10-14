#pragma once

#include "multi_core/core_local.h"

extern CORE_LOCAL(Memory_Allocator * ,imageIOAllocator);

EXTERN_C bool ImageIO_CanSaveAsTGA(Image_ImageHeader const *image);
EXTERN_C bool ImageIO_SaveAsTGA(Image_ImageHeader *image, VFile_Handle handle);

EXTERN_C bool ImageIO_CanSaveAsBMP(Image_ImageHeader const *image);
EXTERN_C bool ImageIO_SaveAsBMP(Image_ImageHeader *image, VFile_Handle handle);

EXTERN_C bool ImageIO_CanSaveAsPNG(Image_ImageHeader const *image);
EXTERN_C bool ImageIO_SaveAsPNG(Image_ImageHeader *image, VFile_Handle handle);

EXTERN_C bool ImageIO_CanSaveAsJPG(Image_ImageHeader const *image);
EXTERN_C bool ImageIO_SaveAsJPG(Image_ImageHeader *image, VFile_Handle handle);

//EXTERN_C bool ImageIO_CanSaveAsHDR(Image_ImageHeader const *image);
//EXTERN_C bool ImageIO_SaveAsHDR(Image_ImageHeader *image, VFile_Handle handle);

EXTERN_C bool ImageIO_CanSaveAsKTX(Image_ImageHeader const *image);
EXTERN_C bool ImageIO_SaveAsKTX(Image_ImageHeader *image, VFile_Handle handle);

EXTERN_C bool ImageIO_CanSaveAsDDS(Image_ImageHeader const *image);
EXTERN_C bool ImageIO_SaveAsDDS(Image_ImageHeader *image, VFile_Handle handle);
