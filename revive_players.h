#pragma once
#include "BLIB\status.h"
#include "dungeon_master.h"
#include "choice_listener.h"

class character;

class revive_players : public BLIB::status, public DM_animator, public choice_listener {
private:

	character* shaman;
	std::vector<character*> dead_players;
	//int sacrifice_pool;
	//int to_sacrifice = 0;
	int revive_index = -1;

	enum {
		sacrifice_select,
		sacrifice_complete
	} sacrifice_state = sacrifice_select;

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
	revive_players(character* shaman, std::vector<character*>& dead_players);
	~revive_players() {}

	void on_hear(int choice) override;

	string				title		() override;
	std::vector<prompt> get_prompts	() override;
	int					show_info	() override;
};