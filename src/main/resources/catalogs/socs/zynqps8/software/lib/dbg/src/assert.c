#include "core/core.h"
#include "dbg/assert.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"

NO_RETURN void assert_printf(char const *file, int line, char const *txt)
{
	debug_printf(DEBUG_RED_PEN "%s:%u: ASSERT %s\n" DEBUG_WHITE_PEN, file, line, txt);
	while(true) {
		; // do nothing
	}
}