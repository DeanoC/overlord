//
// Created by deano on 8/21/22.
//

#pragma once

#include "core/core.h"

void PmuSleep();

void PmuWakeup();

void PmuSafeMemcpy( void *destination, const void *source, size_t num_in_bytes );
