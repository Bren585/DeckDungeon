#include "begin_floor.h"
#include "log.h"

#include "enemy.h"
#include "summoner.h"

#include "assign_floor_buff.h"
#include "cleanup_hand.h"
#include "boss_floor_flag.h"
#include "load_enemy.h"

#define boss 100
#define highest_spawnrate 4

const std::vector<int> spawn_rates[highest_spawnrate] = {
	{0, 7, jack},
	{0, 5, 9, king},
	{0, 2, 6, 10, king},
	{0, 0, 4, 7, 10, king}
};

const string names[13] = {
	"spider",
	"skeleton",
	"zombie",
	"slime",
	"goblin",
	"bat",
	"ghoul",
	"demon",
	"ogre",
	"rat",
	"wolf",
	"troll",
	"daniel"
};

const string boss_names[13] = {
	"giant spider",
	"skeleton king",
	"zombie king",
	"slime king",
	"goblin king",
	"giant bat",
	"ancient ghoul",
	"demon king",
	"ogre chief",
	"rat king",
	"dire wolf",
	"trolololo",
	"cooler daniel"
};

void begin_floor::update(float elapsed_time) {
	auto& players = get_players();

	BEGIN_COROUTINE;

	event_log::record(string("Starting floor ", floor_number));
	
	// Enemy Count
	{ 
		card floor_card = rand_card();
		event_log::record(string("Generating floor with ", floor_card));
		get_scene()->get_pm().add(new card_particle(floor_card), scene_center);

		enemy_count = boss;
		const std::vector<int>& spawn_rate = spawn_rates[players.size() > highest_spawnrate ? highest_spawnrate - 1 : players.size() - 1];
		for (int i = 0; i < spawn_rate.size(); i++) {
			if ((int)floor_card.face() < spawn_rate[i]) {
				enemy_count = i;
				break;
			}
		}

		if (enemy_count == boss) {
			event_log::record("It's a boss floor!");
			enemy_base = {
				4 + floor_number,								// attack
				2 + floor_number,								// defense
				floor_number,									// luck
				10 * int(players.size()) + (3 * floor_number)	// health
			};
			BLIB::manager::add(new boss_floor_flag);
		}
		else {
			event_log::record(string("There are ", enemy_count, " enemies"));
			enemy_base = {
				2 + floor_number,		// attack
				1 + floor_number,		// defense
				0,						// luck
				6 + (2 * floor_number)	// health
			};
		}
	}

	// Enemy Spawning
	COIT = 0;
	SET_CHECKPOINT(0);

	{
		buff_players = false;

		SET_CHECKPOINT(2);
		WAIT(1.0f);

		{
			card buff_card = rand_card();
			get_scene()->get_pm().add(new card_particle(buff_card), scene_center);

			if (buff_card.suit_is(diamond)) {
				buff_players = !buff_players;
				event_log::record(string("Floor buff ", (buff_players ? "goes to a player!" : "returns to the enemy")));
				GO_TO_CHECKPOINT(2);
			}

			stats floor_buff;
			string buff_type;
			switch (buff_card.suit()) {
			case spade:
				floor_buff.attack = (int)ceilf(floor_number / 2.0f);
				buff_type = "attack";
				model_name = "Skeleton_Rogue";
				break;
			case club:
				floor_buff.defense = (int)ceilf(floor_number / 2.0f);
				buff_type = "defense";
				model_name = "Skeleton_Mage";
				break;
			case heart:
				floor_buff.health = floor_number;
				buff_type = "health";
				model_name = "Skeleton_Minion";
				break;
			}

			event_log::record(string(
				"The floor buffs the ",
				buff_type,
				" of ",
				(buff_players ? "a player!" : "the enemy")
			));

			if (enemy_count == boss) { enemy_name = boss_names[buff_card.face() - 1]; model_name = "Skeleton_Warrior"; }
			else { enemy_name = names[buff_card.face() - 1]; }
			string message("A ", enemy_name, " spawned!");
			event_log::record(message);


			enemy_stats = enemy_base;
			if (buff_players) {
				if (players.size() == 1) {
					if (buff_card.suit_is(heart)) {
						players[0]->heal(floor_buff.health);
					}
					else players[0]->buff_for_floor(floor_buff);
					get_scene()->get_pm().add(
						new buff_particles(),
						get_scene()->get_character_model(players[0]->get_id()).get_pos()
					);
				}
				else {
					BLIB::manager::add(new assign_floor_buff(floor_buff));
				}
			}
			else { enemy_stats += floor_buff; }

			if (enemy_count != boss) {
				if (buff_card.face() < 2) {
					enemy_stats.health = 1;
					enemy_stats.attack *= 2;
				}
				else if (buff_card.face() < 4) { enemy_stats.health /= 2; }
				else if (buff_card.face() > 10) { enemy_stats.health += floor_number; }
			}
		}

		YIELD_WHILE(BLIB::manager::find_first_of_type<load_enemy>());
		BLIB::manager::add(new load_enemy(enemy_name, model_name, enemy_stats));
	}

	WAIT(1.0f);

	YIELD_WHILE(BLIB::manager::find_first_of_type<assign_floor_buff>());

	COIT++;
	if (COIT < enemy_count && enemy_count != boss) { GO_TO_CHECKPOINT(0); }

	YIELD_WHILE(BLIB::manager::find_first_of_type<load_enemy>());

	// Player Mulligan / Draw
	COIT = 0;
	SET_CHECKPOINT(1);

	if (!players[COIT]->has_flag(is_thrall)) {
		YIELD_SUBTASK(cleanup_hand, players[COIT]);
	}

	players[COIT]->start_floor();
	if (players[COIT]->has_flag(summons)) {
		players.push_back(new thrall(players[COIT]));
	}

	COIT++;
	if (COIT < players.size()) { GO_TO_CHECKPOINT(1); }

	finish();

	END_COROUTINE;
}