#pragma once

struct MainLoop {
	void Init();
	[[maybe_unused]] void Fini();
	void Loop();

	bool endLoop;
	bool hundredHertzTrigger;
	bool thirtyHertzTrigger;
};

extern MainLoop loopy;
