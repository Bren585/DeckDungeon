#include "end_floor.h"
#include "log.h"

#include "boss_floor_flag.h"
#include "level_up.h"
#include "assign_level_up.h"
#include "revive_players.h"

#include "shaman.h"

void end_floor::update(float elapsed_time) {
	BEGIN_COROUTINE;

	get_scene()->clear_enemies();

	// make everyone relax and despawn summoner's thrall
	{
		for (auto it = get_players().begin(); it != get_players().end(); ) {
			if ((*it)->has_flag(is_thrall)) {
				get_scene()->get_character_model((*it)->get_id()).start_behavior<cb::die>();
				get_scene()->get_character_model((*it)->get_id()).start_behavior<cb::disappear>();
				get_scene()->clear_player((*it)->get_id());
				delete* it;
				it = get_players().erase(it);
				continue;
			}
			if ((*it)->alive()) {
				get_scene()->get_character_model((*it)->get_id()).start_behavior<cb::relax_start>();
			}
			it++;
		}
	}

	WAIT(0.25f);

	// Award levels for clearing the floor;
	{
		boss_flag = BLIB::manager::find_first_of_type<boss_floor_flag>();
		if (boss_flag) {
			BLIB::manager::get_task(boss_flag)->kill();
			event_log::record("It was a boss floor! Everyone gets 3 levels!");
			for (auto& player : get_players()) { player->award_exp(3); }
		}
		else {
			event_log::record("Everyone levels up!");
			for (auto& player : get_players()) { player->award_exp(1); }
		}
	}

	COIT = 0;
	SET_CHECKPOINT(0);

	{
		player = get_players()[COIT];
		if (player->dead()) { 
			GO_TO_CHECKPOINT(1);
		}

		if (player->has_flag(concentrated)) {
			event_log::record(string(player->get_name(), " leveled up via concentration!"));
			extra_levels += 1;
			if (boss_flag) {
				event_log::record(string(player->get_name(), " got extra insight from the boss!"));
				player->award_exp(1);
			}
		}
		
		if (player->has_flag(spiritual)) {
			{
				int bonus = static_cast<shaman*>(player)->redeem_souls();
				if (bonus) {
					event_log::record(string(player->get_name(), " converted ", (bonus / 2) * 5, " souls into ", bonus, " levels!"));
					extra_levels += bonus;
				}

				if (player->get_stats().attack >= 5) {
					std::vector<character*> dead_players;
					for (auto& p : get_players()) { if (p->dead()) dead_players.push_back(p); }
					BLIB::manager::add(new revive_players(player, dead_players));
				}
			}
			YIELD_WHILE(BLIB::manager::find_first_of_type<revive_players>());
		}
	}

	SET_CHECKPOINT(1);
	COIT++;
	if (COIT < get_players().size()) { GO_TO_CHECKPOINT(0); }

	if (extra_levels > 0) {
		YIELD_SUBTASK(assign_level_up, extra_levels);
	}

	// let the dead die
	{
		auto end = std::remove_if(get_players().begin(), get_players().end(), [](character* c) { return c->dead(); });

		for (auto it = end; it != get_players().end(); it++) {
			event_log::record(string("The party gave ", (*it)->get_name(), " a respectful burial"));
			get_scene()->get_character_model((*it)->get_id()).start_behavior<cb::disappear>();
			get_scene()->clear_player((*it)->get_id());
			delete *it;
		}

		get_players().erase(end, get_players().end());
	}

	// Level up
	COIT = 0;
	SET_CHECKPOINT(2);

	get_scene()->spotlight(get_players()[COIT]->get_id(), GREEN);
	YIELD_SUBTASK(level_up, get_players()[COIT]);
	get_scene()->clear_spotlight();

	COIT++;
	if (COIT < get_players().size()) { GO_TO_CHECKPOINT(2); }

	for (auto& player : get_players()) {
		get_scene()->get_character_model(player->get_id()).start_behavior<cb::idle>();
	}

	WAIT(0.5f);

	finish();

	END_COROUTINE;
}