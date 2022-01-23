#pragma once

#if BOARD_myir_fz3 == 1
	#include "myir_fz3/psu_init.h"
#elif BOARD_kv260 == 1
	#include "kv260/psu_init.h"
#endif