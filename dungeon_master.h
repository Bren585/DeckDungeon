#pragma once
#include "BLIB\status.h"
#include "coroutines.h"
#include "character.h"

#include "dungeon_scene.h"
#include "all_behaviors.h"
#include "all_particles.h"

#define GET_DM(dm)	static_cast<		dungeon_master*>(BLIB::manager::get_task	(dm))
#define PEEK_DM(dm) static_cast<const	dungeon_master*>(BLIB::manager::peek_task	(dm))

class dungeon_master;
class dungeon_scene;

class DM_priviledges {
protected:
	const BLIB::task_id dm_id = 0;
	DM_priviledges() : dm_id(BLIB::manager::find_first_of_type<dungeon_master>()) {}
	std::vector<character*>& get_players();
	std::vector<character*>& get_enemies();
};

#define card_spawn_offset float3{0, 2.5f, 0}
class DM_animator {
protected:
	const BLIB::task_id scene_id = 0;
	DM_animator() : scene_id(BLIB::manager::find_first_of_type<dungeon_scene>()) {}
	dungeon_scene* get_scene();
};

class dungeon_master : public BLIB::status, public DM_animator {
	friend DM_priviledges;
private:

	ALLOW_COROUTINES;

	std::vector<character*> players;
	std::vector<character*> enemies;

	int floor_number = 1;

	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override;

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override;
	void on_wake() override {}
	void on_sleep() override {}

public:
	dungeon_master(std::vector<character*> players) : players(players) {}
	~dungeon_master() {}

	const std::vector<character*>& peek_players() const { return players; }
	const std::vector<character*>& peek_enemies() const { return enemies; }
};