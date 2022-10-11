#pragma once

#include "core/core.h"

#ifdef __cplusplus
EXTERN_C {
#endif

// wait for n clocks; clock rate is hw dependent
void Utils_BusyClkSleep(uint64_t clks);

// wait for n micro seconds (1 millionth of a second)
void Utils_BusyMicroSleep(uint64_t useconds);
// wait for n milliseconds (1 thousandth of a second)
void Utils_BusyMilliSleep(uint64_t mseconds);
// wait for n seconds
void Utils_BusySecondSleep(uint64_t seconds);

#ifdef __cplusplus
}
#endif