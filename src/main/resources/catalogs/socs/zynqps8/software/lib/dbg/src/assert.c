#include "core/core.h"
#include "dbg/assert.h"
#include "dbg/print.h"

void assert_printf(char const *file, int line, char const *txt)
{
	debug_printf(DEBUG_RED_PEN "%s:%u: ERROR %s\n", file, line, txt);
}
