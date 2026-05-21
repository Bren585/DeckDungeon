#pragma once
#include "BLIB\status.h"
#include "dungeon_master.h"

constexpr int top_deck			= -1;
constexpr int use_skill			= -2;
constexpr int cancel_skill		= -3;
constexpr int random_action		= -4;

class shaman;

class combat : public BLIB::status, public DM_priviledges, public DM_animator {
private:
	ALLOW_COROUTINES;
	ALLOW_COROUTINE_ITERATOR;

	int choice = 0;
	bool player_phase = false;
	character* attacker = nullptr;
	std::vector<character*> targets;
	pile attacker_discard;
	pile target_discard;

	action attacker_action;
	action defender_action;
	skill attacker_skill;

	shaman* shaman_player = nullptr;

	// Strictly Required for function
	void init() override;
	void update(float elapsed_time) override;

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override {}
	void on_wake() override {}
	void on_sleep() override {}

public:
	combat() {}
	~combat() {}

	void set_choice(int x) { choice = x; }
};
