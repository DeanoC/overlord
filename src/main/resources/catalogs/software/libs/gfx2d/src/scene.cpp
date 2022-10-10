//
// Created by deano on 9/28/22.
//
#include "core/core.h"
#include "dbg/print.h"
#include "gfx2d/scene.hpp"
#include "gfx2d/layer.hpp"

namespace Gfx2d {

void Scene::StartFrame( Gfx2d::Scene *scene ) {

}

void Scene::EndFrame( Gfx2d::Scene *scene ) {
	for(int i = 0; i < scene->numLayers; ++i) {
		auto layer = scene->layers[i];
		if(layer) {
			layer->funcTable->drawFunc(layer, &scene->currentView);
		}
	}
}

void Scene::Destroy(Scene * scene) {
	for(int i = 0; i < scene->numLayers; ++i) {
		auto layer = scene->layers[i];
		if(layer) {
			layer->funcTable->destroyFunc(layer);
			scene->layers[i] = nullptr;
		}
	}
	scene->numLayers - 0;
}

} // end namespace