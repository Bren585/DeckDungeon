#pragma once
#include "BLIB\status.h"
#include "character.h"
#include "choice_listener.h"

class level_up : public BLIB::status, public choice_listener {
private:

	character* player;
	int levels = 0;
	stats to_award;

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
	level_up(character* p);
	~level_up() {}

	void on_hear(int choice) override;

	string				title		() override;
	std::vector<prompt> get_prompts	() override;
	int					show_info	() override { return player->get_id(); }
};