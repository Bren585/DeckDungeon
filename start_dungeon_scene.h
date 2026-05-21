#pragma once
#include "BLIB/scene.h"
#include "character_data.h"

class start_dungeon_scene : public BLIB::flat::scene {
private:
	const std::vector<character_data>& party_data;
	BLIB::task_id scene_id;
	std::vector<class character*> players;

	// Strictly Required for function
	void init() override;
	void update(float elapsed_time) override;
	void draw(BLIB::render_settings) const override {}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override {}
	void on_wake() override {}
	void on_sleep() override {}

public:
	start_dungeon_scene(const std::vector<character_data>& party);
	~start_dungeon_scene() {}
};