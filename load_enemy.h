#pragma once
#include "BLIB\status.h"
#include "begin_floor.h"
#include "enemy.h"
#include <optional>

class load_enemy : public BLIB::status, public DM_priviledges {
private:
	const string enemy_name;
	const string enemy_model;
	const stats enemy_stats;

	std::optional<character_model> model;

	void init() override {
		model.emplace(enemy_model, "none", false);
		//get_enemies().push_back(new enemy(enemy_name, enemy_model, enemy_stats));
	}

	void update(float elapsed_time) override {
		auto* new_enemy = get_enemies().emplace_back(new enemy(enemy_name, enemy_stats));
		get_dungeon_scene()->register_character_model(new_enemy->get_id(), std::move(model.value()), new_enemy);
		finish(); 
	}

public:
	load_enemy(string name, string model, stats stats) : enemy_name(name), enemy_model(model), enemy_stats(stats) {}
	~load_enemy() {}
};