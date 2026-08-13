#pragma once
#include "BLIB/scene.h"
#include "BLIB/camera.h"
#include "character_model.h"
#include "particle_manager.h"
#include "log.h"

class character;

#define scene_center float3{8, 1, 8}

#define get_dungeon_scene() BLIB::manager::get_first_of_type<dungeon_scene>()

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
	void draw_transparent() const override;

	// Optional
	void uninit() override;
	void on_wake() override {}
	void on_sleep() override {}

public:
	dungeon_scene() {}
	~dungeon_scene() {}

	//キャラを登録
	void register_character_model(int id, string filename, string alt_texture, character* character);
	void register_character_model(int id, character_model&& model, character* character);

	character_model& get_character_model(int id) { 
		if (character_models.find(id) != character_models.end()) return character_models[id];
		else return enemy_models[id];
	}

	const character_model& get_character_model(int id) const {
		if (character_models.find(id) != character_models.end()) return character_models.at(id);
		else return enemy_models.at(id);
	}

	//　パーティクルシステムを会得
	particle_manager& get_pm() { return pm; }

	void clear_enemies() { enemy_models.clear(); }

	void clear_player(int id) { graveyard.push_back(id); }

	//　キャラに光を
	void spotlight(int it, color c = WHITE);

	void clear_spotlight();

};