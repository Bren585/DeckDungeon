#pragma once
#include "BLIB\status.h"
#include "dungeon_master.h"

class begin_floor : public BLIB::status, public DM_priviledges, public DM_animator {
private:
	ALLOW_COROUTINES;
	ALLOW_COROUTINE_ITERATOR;

	int floor_number;
	int enemy_count = 0;
	stats enemy_base;
	stats enemy_stats;
	string enemy_name;
	string model_name;

	bool buff_players = false;

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
	begin_floor(int floor) : floor_number(floor) {}
	~begin_floor() {}
};