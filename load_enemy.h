#pragma once
#include "BLIB\status.h"
#include "begin_floor.h"
#include "enemy.h"
class load_enemy : public BLIB::status, public DM_priviledges {
private:
	const string enemy_name;
	const string enemy_model;
	const stats enemy_stats;

	// Strictly Required for function
	void init() override {
		get_enemies().push_back(new enemy(enemy_name, enemy_model, enemy_stats));
	}

	void update(float elapsed_time) override { finish(); }

public:
	load_enemy(string name, string model, stats stats) : enemy_name(name), enemy_model(model), enemy_stats(stats) {}
	~load_enemy() {}
};