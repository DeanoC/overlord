#include "core/core.h"
#include "dbg/assert.h"
#include "dbg/raw_print.h"
#include "dbg/ansi_escapes.h"

void assert_printf(char const *file, int line, char const *txt)
{
	raw_debug_printf(DEBUG_RED_PEN "%s:%u: ASSERT %s\n" DEBUG_WHITE_PEN, file, line, txt);
}