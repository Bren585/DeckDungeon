#include "dungeon_master.h"
#include "dungeon_scene.h"
#include "dungeon_overlay.h"
#include "title_scene.h"
#include "log.h"
#include "BLIB\transition_scene.h"

#include "begin_floor.h"
#include "combat.h"
#include "end_floor.h"

std::vector<character*>& DM_priviledges::get_players() { return GET_DM(dm_id)->players; }
std::vector<character*>& DM_priviledges::get_enemies() { return GET_DM(dm_id)->enemies; }

dungeon_scene* DM_animator::get_scene() { return static_cast<dungeon_scene*>(BLIB::manager::get_scene(scene_id)); }

constexpr int dungeon_start		= 0;
constexpr int dungeon_survive	= 1;
constexpr int dungeon_dead		= 2;
constexpr int dungeon_end		= 3;

void dungeon_master::update(float elapsed_time) {
	BEGIN_COROUTINE;

	YIELD_WHILE(BLIB::manager::get_scene(BLIB::manager::find_first_of_type<dungeon_scene>())->report() != active);
	
	SET_CHECKPOINT(dungeon_start);

	if (rest_bgm != BLIB::audio::unset) {
		BLIB::audio::stop(rest_bgm, 1.0f);
		rest_bgm = BLIB::audio::unset;
	}
	combat_bgm = BLIB::audio::play("battle", 1.0f, true);

	WAIT(2.0f);

	YIELD_SUBTASK(begin_floor, floor_number);

	event_log::record("Finished floor generation");

	YIELD_SUBTASK(combat);

	if (combat_bgm != BLIB::audio::unset) {
		BLIB::audio::stop(combat_bgm, 1.0f);
		combat_bgm = BLIB::audio::unset;
	}

	{
		bool survived = false;
		for (auto& player : players) { if (player->alive()) { survived = true; break; } }

		if (survived) {
			GO_TO_CHECKPOINT(dungeon_survive);
		}
		else {
			GO_TO_CHECKPOINT(dungeon_dead);
		}
	}

	SET_CHECKPOINT(dungeon_survive); // survived

	rest_bgm = BLIB::audio::play("rest", 1.0f, true);

	event_log::record(string("Floor ", floor_number, " cleared!"));
	for (auto& player : players) {
		if (player->alive()) {
			get_scene()->get_character_model(player->get_id()).start_behavior<cb::celebrate>();
		}
	}
	WAIT(1.0f);

	YIELD_SUBTASK(end_floor);

	BLIB::manager::unstage(scene_id, BLIB::transition::fade, 1.0f);
	YIELD_WHILE(BLIB::manager::find_first_of_type<BLIB::transition_scene>());
	BLIB::manager::stage(scene_id, 1, BLIB::transition::fade, 1.0f);

	GO_TO_CHECKPOINT(dungeon_start);

	SET_CHECKPOINT(dungeon_dead); // game over

	BLIB::audio::play("tpk", 1.0f, true);

	event_log::record("Game over");
	event_log::record(string("Your party survived ", floor_number, " floors"));

	YIELD_WHILE(!BLIB::input::trigger());

	SET_CHECKPOINT(dungeon_end);

	BLIB::manager::unstage(scene_id, BLIB::transition::fade, 1.0f);
	BLIB::manager::unstage(BLIB::manager::find_first_of_type<dungeon_overlay>(), BLIB::transition::fade, 1.0f);
	get_scene()->stop();
	get_overlay()->stop();

	YIELD_WHILE(BLIB::manager::find_first_of_type<BLIB::transition_scene>());

	BLIB::audio::stop(BLIB::audio::all_tracks, 1.0f);
	BLIB::manager::stage(BLIB::manager::find_first_of_type<title_scene>(), 0, BLIB::transition::fade, 0.5f);
	finish();

	END_COROUTINE;
}

void dungeon_master::kill() { 
	for (auto& player : players) { delete player; } 
	for (auto& enemy : enemies) { delete enemy; } 
}