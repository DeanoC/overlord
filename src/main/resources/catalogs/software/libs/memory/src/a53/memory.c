#include "memory/memory.h"

Memory_Allocator Memory_GlobalAllocator = { 0
//        .aalloc = platform_aligned_alloc,
//       .calloc = platform_calloc,
//        .free = platform_free,
//        .malloc = platform_malloc,
//        .realloc = platform_realloc
};
