#include "core/core.h"
#include "memory/memory.h"
#include "multi_core/core_local.h"
#include "tiny_image_format/tiny_image_format_base.h"

extern CORE_LOCAL(Memory_Allocator * ,stbIoAllocator);

#define STBI_MALLOC(size)         MALLOC(stbIoAllocator, size)
#define STBI_REALLOC(p,newsz)     MREALLOC(stbIoAllocator, p,newsz)
#define STBI_FREE(p)              MFREE(stbIoAllocator, p)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYKTX_HAVE_MEMCPY 1
#define TINYKTX_IMPLEMENTATION 1
#include "tiny_ktx.h"

#define TINYDDS_HAVE_MEMCPY 1
#define TINYDDS_IMPLEMENTATION 1
#include "tiny_dds.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
