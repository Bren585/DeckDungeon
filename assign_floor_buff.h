#pragma once
#include "BLIB\status.h"
#include "dungeon_master.h"
#include "choice_listener.h"

class assign_floor_buff : public BLIB::status, public DM_priviledges, public DM_animator, public choice_listener {
private:

	stats floor_buff;

	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override {}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override {}
	void on_wake() override;
	void on_sleep() override {}

public:
	assign_floor_buff(stats buff) : floor_buff(buff) {}
	~assign_floor_buff() {}

	void on_hear(int choice) override;

	string				title		() override;
	std::vector<prompt> get_prompts	() override;
};