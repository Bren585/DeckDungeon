#pragma once
#include "cards.h"
#include "stats.h"
#include "BLIB/string.h"

class character;

enum action_flags : int {
	no_action_flags			= 0,
	target_ally				= 1 << 1,
	target_enemy			= 1 << 2,
	can_target_self			= 1 << 3,
	target_all_allies		= 1 << 4,
	target_all_enemies		= 1 << 5,
 	attack_bonus			= 1 << 6,
	defense_bonus			= 1 << 7,
	act_again				= 1 << 8,
	is_combat				= 1 << 9,
	is_turn_buff			= 1 << 10,
	is_round_buff			= 1 << 11,
	is_floor_buff			= 1 << 12,
	cannot_be_buffed		= 1 << 13,
	heal_bonus				= 1 << 14,
	target_thrall_only		= 1 << 15,
	target_dead_allies		= 1 << 16,
	magic_attack			= 1 << 17,

	is_buff			= is_turn_buff | is_round_buff | is_floor_buff,

	is_turn_debuff	= is_turn_buff,
	is_round_debuff = is_round_buff,
	is_floor_debuff = is_floor_buff,
	is_debuff		= is_buff,

	is_heal			= is_floor_buff,
	is_damage		= is_floor_buff,

	single_target	= target_ally | target_enemy | can_target_self,
	multi_target	= target_all_allies | target_all_enemies,

	can_kill		= target_enemy | target_all_enemies, 
};

struct action {
	card cost;
	action_flags flags = no_action_flags;
	stats buff;
	string name;

	action() = default;
	action(card c, int f, stats b, string n) : cost(c), flags(action_flags(f)), buff(b), name(n) {}
};