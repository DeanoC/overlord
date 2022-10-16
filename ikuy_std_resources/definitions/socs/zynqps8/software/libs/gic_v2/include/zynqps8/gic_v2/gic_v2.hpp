#pragma once

#include "core/core.h"
#include "platform/zynqmp/interrupts.h"

namespace GicV2 {

void InitDist();
void InitCPU();

void EnableInterruptForThisCore(Interrupt_Names name_);
void DisableInterruptForThisCore(Interrupt_Names name_);

} // end namespace