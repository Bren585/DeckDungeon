#pragma once
#include "BLIB\status.h"
#include "choice_listener.h"

class combat;
class character;

class choose_target : public BLIB::status, public choice_listener {
private:

	combat* parent;
	const std::vector<character*>& targets;

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
	choose_target(combat* ptr, const std::vector<character*>& targets) : parent(ptr), targets(targets) {}
	~choose_target() {}

	void on_hear(int choice) override;

	string				title		() override;
	std::vector<prompt> get_prompts	() override;
};