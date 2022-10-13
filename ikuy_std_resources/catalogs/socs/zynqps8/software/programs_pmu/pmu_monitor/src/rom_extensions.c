#include "core/core.h"
#include "rom_extensions.h"

__attribute__((__section__(".RomExtensionTable")))
RomServiceExtensionHandler RomExtensionTable[REN_TBL_MAX] __attribute__((used));

