#pragma once
#include "BLIB\status.h"
class boss_floor_flag : public BLIB::status {
private:

	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override {}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override { finish(); }
	void on_wake() override {}
	void on_sleep() override {}

public:
	boss_floor_flag() {}
	~boss_floor_flag() {}
};