
#include "core/core.h"
#include <cstdio>
#undef dbg
#define BACKWARD_HAS_DW 1
#include "platform/host/backward.hpp"

EXTERN_C void assert_printf(char const *file, int line, char const *txt)
{
	printf("\n %s:%u: ASSERT %s\n", file, line, txt);

	using namespace backward;
	StackTrace st; st.load_here(32);
	Printer p; p.print(st);

	while(true) {
		; // do nothing
	}
}