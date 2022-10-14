#pragma once

namespace Timers {
static const int MaxHundredHzCallbacks = 4;
	static const int MaxThirtyHzCallbacks = 4;

	typedef void (*Callback)();

	void Init();
	void Start();

}
