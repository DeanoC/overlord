#pragma once

#include "core/core.h"

namespace Dma::ZDma {

void SimpleDmaSet32(uintptr_lo_t baseAddr_, uint32_t data_, uintptr_all_t address_, uint32_t size_);

void SimpleDmaCopy(uintptr_lo_t baseAddr_, uintptr_all_t src_, uintptr_all_t dest_, uint32_t size_);

void Stall(uintptr_lo_t baseAddr_);

} // end namespace