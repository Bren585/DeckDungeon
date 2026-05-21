#pragma once
#pragma once
#include "character.h"

class swordsman : public character {
	action basic_attack(card card) override {
		switch (card.suit()) {
		case spade:		return ACTION(target_enemy | is_combat | attack_bonus | is_damage, (0, 0, (int)std::ceilf(-get_stats().attack / 3.0f), 0), "double attacks");
		case club:		return DEFAULT_ATTACK;
		case diamond:	return DEFAULT_LUCK;
		case heart:		return DEFAULT_HEAL;
		default:
			assert(false);
			return action();
		}
	}

	action reaction(card card) override {
		switch (card.suit()) {
		case spade:		return DEFAULT_WHIFF;
		case club:		return DEFAULT_DEFEND;
		case diamond:	return DEFAULT_LUCK;
		case heart:		return DEFAULT_HEAL;
		default:
			assert(false);
			return action();
		}
	}

public:
	swordsman(string name) : character(name, stats(6, 3, 1, 9), character_flags::none) {
		make_model("Barbarian", "A");
		skills = {
			
		};
	}
};