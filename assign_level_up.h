#pragma once
#include "BLIB\status.h"
#include "dungeon_master.h"
#include "choice_listener.h"

class assign_level_up : public BLIB::status, public DM_priviledges, public DM_animator, public choice_listener {
private:

	int to_assign;
	//std::vector<int> assigned;

	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override {}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override { wake(); }

	// Optional
	void kill() override {}
	void on_wake() override;
	void on_sleep() override {}

public:
	assign_level_up(int levels) : to_assign(levels) { /*assigned.resize(get_players().size(), 0);*/ }
	~assign_level_up() {}

	void on_hear(int choice) override;

	string				title		() override;
	std::vector<prompt> get_prompts	() override;
};