//
// Created by deano on 9/27/22.
//

#pragma once

#include "gfx2d/base.hpp"

namespace Gfx2d {

struct Layer;

// layer 0 should be a solid layer
struct Scene {
	static constexpr uint8_t const MAX_LAYERS = 16;
	Size2d sceneDimensions;
	uint8_t numLayers;
	Layer * layers[MAX_LAYERS];

	Viewport currentView;

	static void StartFrame(Scene * scene);
	static void EndFrame(Scene * scene);

	static void Destroy(Scene * scene);

};


} // end namespace