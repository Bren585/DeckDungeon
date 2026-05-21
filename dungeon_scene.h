#pragma once
#include "BLIB/scene.h"
#include "BLIB/camera.h"
#include "character_model.h"
#include "particle_manager.h"
#include "log.h"

class character;

#define scene_center float3{8, 1, 8}

class dungeon_scene : public BLIB::full::scene {
private:
	BLIB::perspective_camera cam;

	std::unordered_map<int, character_model>	character_models;
	std::unordered_map<int, character_model>	enemy_models;
	std::vector<int>							graveyard;
	BLIB::full::object backdrop;

	particle_manager pm;

	// Strictly Required for function
	void init() override;

	void update(float elapsed_time) override;

	void draw(BLIB::render_settings rs) const override {
		for (auto& it : character_models	) { it.second.render(rs); } 
		for (auto& it : enemy_models		) { it.second.render(rs); }
		backdrop.render(rs);
	}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override;
	void draw_transparent() const override { pm.render({}); }

	// Optional
	void kill() override;
	void on_wake() override {}
	void on_sleep() override {}

public:
	dungeon_scene() {}
	~dungeon_scene() {}

	void register_character_model(int id, string filename, string alt_texture, character* character);
	character_model& get_character_model(int id) { 
		if (character_models.find(id) != character_models.end()) return character_models[id];
		else return enemy_models[id];
	}

	particle_manager& get_pm() { return pm; }

	void clear_enemies() { enemy_models.clear(); }

	void clear_player(int id) { graveyard.push_back(id); }

	void spotlight(int it, color c = WHITE);

	void clear_spotlight();

	void stop();
};