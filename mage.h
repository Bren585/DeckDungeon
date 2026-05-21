#pragma once
#include "character.h"
class mage : public character {
	action basic_attack(card card) override {
		switch (card.suit()) {
		case spade:		return DEFAULT_ATTACK;
		case club:		return DEFAULT_WHIFF;
		case diamond:	return action(card, target_enemy | is_combat | attack_bonus | magic_attack, stats{}, "blasts");
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
		case diamond:	return action(card, defense_bonus, stats{}, "casts a protection spell");
		case heart:		return DEFAULT_HEAL;
		default:
			assert(false);
			return action();
		}
	}

public:
	mage(string name) : character(name, stats(5, 4, 1, 9), character_flags::none) {
		make_model("Mage", "none");
		skills = {
			skill(
				"Fireball",
				"Casts a fireball that does damage to all enemies",
				diamond,
				[](card cost, character* c) { return action(
					cost,
					target_all_enemies |
					magic_attack | 
					is_damage,
					stats(0, 0, 0, (int) -std::ceilf(c->get_buffed_stats().attack / 3.0f)),
					"fireballs"
				); }
			),
		};
	}
};