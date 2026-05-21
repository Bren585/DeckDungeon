#pragma once
#include "BLIB\status.h"
#include "dungeon_master.h"

class end_floor : public BLIB::status, public DM_priviledges, public DM_animator {
private:
	ALLOW_COROUTINES;
	ALLOW_COROUTINE_ITERATOR;

	character* player = nullptr;
	BLIB::task_id boss_flag = 0;
	int choice = 0;
	int extra_levels = 0;

	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override;

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override {}
	void on_wake() override {}
	void on_sleep() override {}

public:
	end_floor() {}
	~end_floor() {}

	void set_choice(int x) { choice = x; }
};
