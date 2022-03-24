#include "core/core.h"
#include "dbg/assert.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"

#if CPU_pmu != 1 && CPU_host != 1
NO_RETURN void assert_printf(char const *file, int line, char const *txt)
{
	debug_printf(ANSI_RED_PEN "\n %s:%u: ASSERT %s\n" ANSI_WHITE_PEN, file, line, txt);
	while(true) {
		; // do nothing
	}
}

#endif