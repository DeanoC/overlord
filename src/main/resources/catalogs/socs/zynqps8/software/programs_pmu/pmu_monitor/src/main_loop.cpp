#include "core/core.h"
#include "main_loop.hpp"
#include "os_heap.hpp"

void MainLoop::Init() {
	endLoop = false;
	ThirtyHertzTrigger = false;
}

void MainLoop::Loop() {
	while(!endLoop) {
		if(ThirtyHertzTrigger) {
			for(auto & callback : osHeap->ThirtyHzCallbacks) {
				if(callback != nullptr) {
					callback();
				}
			}
			ThirtyHertzTrigger = false;
		}
	}
}