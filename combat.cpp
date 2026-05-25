#include "combat.h"
#include "log.h"

#include "shaman.h"

#include "choose_action.h"
#include "choose_target.h"
#include "choose_skill.h"

enum combat_checkpoint {
	start_round,
	start_turn,
	start_action,
	start_reaction,
	killed_self,
	end_action,
	end_turn,
	end_round,
	end_floor,
};

void combat::init() {
	for (auto& player : get_players()) {
		if (player->has_flag(spiritual)) {
			shaman_player = static_cast<shaman*>(player);
			break;
		}
	}
}

void combat::update(float elapsed_time) {
	auto& players = get_players();
	auto& enemies = get_enemies();
	auto& attackers = player_phase ? players : enemies;
	auto& defenders = player_phase ? enemies : players;

	BEGIN_COROUTINE;

	SET_CHECKPOINT(start_round);

	player_phase = !player_phase;

	event_log::record("New round begins!");
	event_log::record(player_phase ? "Player Phase" : "Enemy Phase");

	WAIT(0.25f);

	COIT = 0;
	SET_CHECKPOINT(start_turn);

	attacker = attackers[COIT];

	if (attacker->dead()) {
		GO_TO_CHECKPOINT(end_turn);
	}

	event_log::record(string(attacker->get_name(), "'s turn"));
	attacker_discard.empty();
	target_discard.empty();
	SET_CHECKPOINT(start_action);

	get_scene()->spotlight(attacker->get_id());

	if (attacker->has_flag(cannot_attack)) {
		if (attacker->get_hand().size() == 0) {
			event_log::record(string(attacker->get_name(), " cannot act!"));
			GO_TO_CHECKPOINT(end_turn);
		}
		choice = use_skill;
	} 
	else if (attacker->has_flag(mindless)) {
		choice = random_action;
	}
	else { 
		YIELD_SUBTASK(choose_action, this, attacker->get_id(), action_any);
	}

	// Choosing action
	{
		if (choice == use_skill) {
			YIELD_SUBTASK(choose_skill, this, attacker->get_skills());
			if (choice == cancel_skill) { GO_TO_CHECKPOINT(start_action); }
			attacker_skill = attacker->get_skills()[choice];

			if (attacker_skill.cost != no_suit) {
				action_type cost;
				if		(attacker_skill.cost == spade)		cost = action_pay_spade;
				else if (attacker_skill.cost == heart)		cost = action_pay_heart;
				else if (attacker_skill.cost == club)		cost = action_pay_club;
				else if (attacker_skill.cost == diamond)	cost = action_pay_diamond;
				else										cost = action_pay_any;
				YIELD_SUBTASK(choose_action, this, attacker->get_id(), cost);
				if (choice == cancel_skill) { GO_TO_CHECKPOINT(start_action); }
			}
			attacker_action = attacker_skill.effect(attacker->get_hand().play(choice), attacker);
			choice = use_skill;
		}

		if (choice == top_deck) {
			attacker_action = attacker->basic_attack(attacker->get_deck().play());
		}
		else if (choice == random_action) {
			attacker_action = attacker->basic_attack(rand_card());
		}
		else if (choice >= 0) {
			attacker_action = attacker->basic_attack(attacker->get_hand().play(choice));
		}
	}

	get_scene()->get_pm().add(new card_particle(attacker_action.cost), get_scene()->get_character_model(attacker->get_id()).get_pos() + card_spawn_offset);
	get_scene()->clear_spotlight();

	// Targeting
	{
		// Collect potential targets
		targets.clear();
		if (attacker_action.flags & (target_ally | target_all_allies)) {
			for (auto& ally : attackers) {
				if (attacker_action.flags & target_dead_allies) {
					if (ally->alive()) continue;
				}
				else {
					if (ally->dead()) continue;
				}

				if (attacker_action.flags & target_thrall_only) {
					if (!(ally->has_flag(is_thrall))) continue;
				}
				
				if (*ally != *attacker || attacker_action.flags & can_target_self) {
					targets.push_back(ally);
				}
			}
		}
		else if (attacker_action.flags & (target_enemy | target_all_enemies)) {
			for (auto& defender : defenders) { 
				if (defender->dead()) continue;
				if (defender->has_flag(cannot_attack)) continue;
				targets.push_back(defender); 
			}
		}
		else if (attacker_action.flags & can_target_self) {
			targets.push_back(attacker);
		}
		else {
			assert(false); // No targeting flags
		}

		// No valid targets
		if (targets.size() == 0) {
			if (!attacker->has_flag(mindless)) {
				if (choice == top_deck) { pile({ attacker_action.cost }).into(attacker_discard); } 
				else { pile({ attacker_action.cost }).into(attacker->get_hand()); }
			}
			event_log::record("Card could not be played.");
			GO_TO_CHECKPOINT(start_action);
		}

		// Pick target from valid targets
		if (!(attacker_action.flags & multi_target)) {

			if (targets.size() == 1) choice = 0;
			else if (attacker->has_flag(mindless)) {
				choice = -1;
				{
					int target_evasion = 1000;
					for (int i = 0; i < targets.size(); i++) {

						int evasion;
						if (targets[i]->has_flag(mindless)) {
							card c = rand_card();
							evasion = targets[i]->get_luck_bonus() + c.face() + targets[i]->get_buffs().luck;
							get_scene()->get_pm().add(new card_particle(c), get_scene()->get_character_model(targets[i]->get_id()).get_pos() + card_spawn_offset);
						}
						else {
							pile temp;
							targets[i]->get_deck().draw().into(temp);
							evasion = targets[i]->get_luck_bonus() + card(temp).face() + targets[i]->get_buffs().luck;
							get_scene()->get_pm().add(new card_particle(temp), get_scene()->get_character_model(targets[i]->get_id()).get_pos() + card_spawn_offset);
							temp.into(targets[i]->get_deck());
							targets[i]->get_deck().shuffle();
						}

						if (evasion < target_evasion) {
							choice = i;
							target_evasion = evasion;
						}
					}
				}
				assert(choice != -1);
				WAIT(0.5f);
			}
			else {
				YIELD_SUBTASK(choose_target, this, targets);
			}

			targets = { targets[choice] };
			event_log::record(string(
				attacker->get_name(),
				" ",
				attacker_action.name,
				" ",
				*targets[0] == *attacker ? string("self") : targets[0]->get_name()
			));
		}
		else {
			event_log::record(string(
				attacker->get_name(),
				" ",
				attacker_action.name,
				" ",
				attacker_action.flags & target_all_allies ? "all allies" : "all enemies"
			));
		}
	}

	// Buff effects
	{
		if (attacker_action.flags & is_buff) {
			get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::magic_skill>();
			WAIT(0.1f);
			YIELD_WHILE(!get_scene()->get_character_model(attacker->get_id()).behavior_is<cb::idle>());

			{
				stats buff = attacker_action.buff;
				bool is_debuff = false;
				for (int i = 0; i < 4; i++) {
					int& stat = buff[i];
					if (stat == 0)	continue;
					if (stat > 0) { is_debuff = false;  break; }
					else		  { is_debuff = true;	break; }
				}

				if ((attacker_action.flags & cannot_be_buffed) == 0) {
					int luck_bonus = attacker->get_buffs().luck * is_debuff ? -1 : 1;
					for (int i = 0; i < 4; i++) { if (buff[i] != 0) buff[i] += luck_bonus; }
				}

				for (auto target : targets) {
					if		(attacker_action.flags & is_turn_buff ) { target->buff_for_turn (buff); }
					else if (attacker_action.flags & is_round_buff) { target->buff_for_round(buff); }
					else if (attacker_action.flags & is_floor_buff) { target->buff_for_floor(buff); }

					if (attacker_action.flags & target_dead_allies && target->alive()) {
						get_scene()->get_character_model(target->get_id()).start_behavior<cb::relax_end>("Lie_");
					}
				}

				for (auto target : targets) {
					if (attacker_action.flags & magic_attack) {
						get_scene()->get_pm().add(
							new magic_attack_particles(),
							get_scene()->get_character_model(target->get_id()).get_pos()
						);
					}
					else {
						if (is_debuff) {
							get_scene()->get_pm().add(
								new debuff_particles(),
								get_scene()->get_character_model(target->get_id()).get_pos()
							);
						}
						else {
							get_scene()->get_pm().add(
								new buff_particles(), 
								get_scene()->get_character_model(target->get_id()).get_pos()
							);
						}
					}
				}
			}			
		}
	}

	// Do combat
	if (attacker_action.flags & is_combat) {
		get_scene()->spotlight(targets[0]->get_id(), RED);
		// Enemy Reaction
		SET_CHECKPOINT(start_reaction);

		if (targets[0]->has_flag(mindless)) {
			choice = random_action;
		}
		else {
			YIELD_SUBTASK(choose_action, this, targets[0]->get_id(), action_reaction);
		}

		if (choice == top_deck) {
			defender_action = targets[0]->reaction(targets[0]->get_deck().play());
		}
		else if (choice == random_action) {
			defender_action = targets[0]->reaction(rand_card());
		}
		else if (choice >= 0) {
			defender_action = targets[0]->reaction(targets[0]->get_hand().play(choice));
		}

		event_log::record(string(targets[0]->get_name(), " ", defender_action.name));

		if (choice != random_action) {
			pile({ defender_action.cost }).into(target_discard);
		}

		get_scene()->get_pm().add(new card_particle(defender_action.cost), get_scene()->get_character_model(targets[0]->get_id()).get_pos() + card_spawn_offset);
		get_scene()->clear_spotlight();
		
		if (defender_action.flags & is_turn_buff && defender_action.flags & can_target_self) {
			targets[0]->buff_for_turn(defender_action.buff);
			get_scene()->get_pm().add(
				new buff_particles(),
				get_scene()->get_character_model(targets[0]->get_id()).get_pos()
			);
			get_scene()->get_character_model(targets[0]->get_id()).start_behavior<cb::magic_skill>();
			WAIT(0.1f);
			YIELD_WHILE(!get_scene()->get_character_model(targets[0]->get_id()).behavior_is<cb::idle>());
			if (defender_action.flags & act_again) {
				GO_TO_CHECKPOINT(start_reaction);
			}
		}

		if (!targets[0]->has_flag(mindless)) {
			target_discard.into(targets[0]->get_deck());
			targets[0]->get_deck().shuffle();
		}

		{
			int attack = attacker->get_buffed_stats().attack;
			if (attacker_action.flags & heal_bonus) attacker->heal(attacker_action.cost.modifier() + attacker->get_buffs().luck);
			if (attacker_action.flags & attack_bonus) attack += attacker_action.cost.modifier() + attacker->get_buffs().luck;

			if (attacker->dead()) {
				event_log::record(string(attacker->get_name(), " tripped and died!"));
				if (shaman_player) { shaman_player->gain_souls(1); }
				attacker->on_death();
				targets[0]->end_turn();

				GO_TO_CHECKPOINT(killed_self);
			}

			int defense = targets[0]->get_buffed_stats().defense;
			if (defender_action.flags & heal_bonus) targets[0]->heal(defender_action.cost.modifier() + targets[0]->get_buffs().luck);
			if (defender_action.flags & defense_bonus) { 
				defense += defender_action.cost.modifier() + targets[0]->get_buffs().luck; 
				get_scene()->get_character_model(targets[0]->get_id()).start_behavior<cb::block>();
			}

			int damage = attack - defense;
			if (damage < 0) damage = 0;
			event_log::record(string("Dealt ", damage, " damage in combat"));
			targets[0]->damage(damage);
			targets[0]->end_turn();
		}

		if (attacker_action.flags & magic_attack) {
			get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::magic_attack>(&get_scene()->get_character_model(targets[0]->get_id()));
		}
		else {
			get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::melee_attack>(&get_scene()->get_character_model(targets[0]->get_id()));
		}

		WAIT(0.25f);
		YIELD_WHILE(!get_scene()->get_character_model(attacker->get_id()).behavior_is<cb::idle>());
		get_scene()->get_character_model(targets[0]->get_id()).start_behavior<cb::idle>();
	}

	// check for deaths / award EXP
	if (attacker_action.flags & can_kill) {
		for (auto defender_it = defenders.begin(); defender_it != defenders.end(); ) {
			if ((*defender_it)->dead()) {
				if (get_scene()->get_character_model((*defender_it)->get_id()).behavior_scheduled<cb::stay_dead>()) { defender_it++; continue; }
				get_scene()->get_character_model((*defender_it)->get_id()).start_behavior<cb::die>();
				event_log::record(string((*defender_it)->get_name(), " died!"));
				attacker->award_exp(1);
				if (shaman_player) { shaman_player->gain_souls(1); }
				if (player_phase) {
					get_scene()->get_character_model((*defender_it)->get_id()).start_behavior<cb::disappear>();
					delete (*defender_it);
					defender_it = defenders.erase(defender_it);
					continue;
				}
			}
			defender_it++;
		}
	}

	GO_TO_CHECKPOINT(end_action);

	// Accidentally died in attack phase
	SET_CHECKPOINT(killed_self);

	get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::die>();
	YIELD_WHILE(get_scene()->get_character_model(attacker->get_id()).behavior_scheduled<cb::die>());

	if (!player_phase) {
		get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::disappear>();
		delete attacker;
		enemies.erase(enemies.begin() + COIT--);
		GO_TO_CHECKPOINT(end_turn);
	}

	// End turn
	SET_CHECKPOINT(end_action);
	if (!attacker->has_flag(mindless)) {
		pile({ attacker_action.cost }).into(attacker_discard);
	}
	if (attacker_action.flags & act_again && enemies.size() > 0 && attacker->alive()) { GO_TO_CHECKPOINT(start_action); }
	attacker->end_turn();
	if (!attacker->has_flag(mindless)) {
		attacker_discard.into(attacker->get_deck());
		attacker->get_deck().shuffle();
	}

	SET_CHECKPOINT(end_turn);
	if (player_phase && attacker->alive()) {
		if (attacker->get_hand().size() < 3 && !attacker->has_flag(is_thrall)) {
			attacker->get_deck().draw(1).into(attacker->get_hand());
		}
	}

	// clear check
	{
		bool all_dead = true;
		for (auto& defender : defenders) { if (defender->alive()) { all_dead = false; break; } }
		if (all_dead) { GO_TO_CHECKPOINT(end_floor); }
	}

	WAIT(0.5f);

	COIT++;
	if (COIT < attackers.size()) { GO_TO_CHECKPOINT(start_turn); }

	if (player_phase) { GO_TO_CHECKPOINT(start_round); }

	// End round
	SET_CHECKPOINT(end_round);

	for (auto& player : players) { player->end_round(); }
	for (auto& enemy : enemies) { enemy->end_round(); }

	GO_TO_CHECKPOINT(start_round);

	SET_CHECKPOINT(end_floor);

	for (auto& player : players) { player->end_floor(); }

	finish();
	END_COROUTINE;
}