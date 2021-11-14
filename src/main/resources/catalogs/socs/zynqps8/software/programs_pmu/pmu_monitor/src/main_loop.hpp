#pragma once
struct MainLoop {
	void Init();
	void Loop();

	bool endLoop;
	bool ThirtyHertzTrigger;
};

extern MainLoop loopy;
