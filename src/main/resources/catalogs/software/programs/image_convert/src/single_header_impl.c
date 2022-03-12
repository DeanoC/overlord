#include "core/core.h"
#include "tiny_image_format/tiny_image_format_base.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYKTX_HAVE_MEMCPY 1
#define TINYKTX_IMPLEMENTATION 1
#include "tiny_ktx.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
