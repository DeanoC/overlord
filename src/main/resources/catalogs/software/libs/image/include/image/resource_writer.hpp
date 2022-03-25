//
// Created by deano on 3/17/22.
//

#pragma once

#include "library_defines/library_defines.h"

#if IKUY_HAVE_LIB_RESOURCE_BUNDLE_WRITER == 1
#include "data_binify/write_helper.hpp"

void ImageChunkWriter(void * userData_, Binify::WriteHelper& helper);
#endif