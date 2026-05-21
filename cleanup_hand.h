#pragma once
#include "BLIB\status.h"
#include "character.h"
#include "choice_listener.h"

class cleanup_hand : public BLIB::status, public choice_listener {
private:

	character* player;
	pile mulligan;
	bool discarded = false;
	//bool discard[3] = { false };

	// Strictly Required for function
	void init() override;
	void update(float elapsed_time) override {}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override { wake(); }

	// Optional
	void kill() override {}
	void on_wake() override;
	void on_sleep() override {}

	void end_cleanup();

public:
	cleanup_hand(character* player) : player(player) {}
	~cleanup_hand() {}

	void on_hear(int choice) override;

	string				title		() override { return discarded ? "Mulligan your hand" : "Discard unwanted cards"; }
	int					show_info	() override { return player->get_id(); }
	bool				allow_cards	() override { return true; }
	std::vector<prompt> get_prompts	() override { return {prompt("Finish", -1)}; }
};