//
// Created by deano on 9/27/22.
//

#pragma once
#include "gfx2d/base.hpp"
#include "memory/memory.h"

namespace Gfx2d {

struct Layer;

typedef void (*LayerDestroyFunc)(Layer * layer);
typedef void (*LayerDrawFunc)(Layer * layer, Viewport const * view);

struct LayerFuncTable {
	LayerDestroyFunc destroyFunc;
	LayerDrawFunc drawFunc;
};

struct Layer {
	LayerFuncTable * funcTable;
	Memory_Allocator * allocator;
};



}