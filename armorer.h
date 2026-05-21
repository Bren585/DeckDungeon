#pragma once
#include "character.h"

class armorer : public character {
	action basic_attack(card card) override {
		switch (card.suit()) {
		case spade:		return DEFAULT_ATTACK;
		case club:		return DEFAULT_WHIFF;
		case diamond:	return DEFAULT_LUCK;
		case heart:		return ACTION(target_enemy | heal_bonus | is_combat, (0, 0, 0, card.modifier() + 1), "heals carefully before attacking");
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
		case heart:		return ACTION(target_enemy | heal_bonus | is_combat, (0, 0, 0, card.modifier() + 1), "heals carefully before defending");
		default:
			assert(false);
			return action();
		}
	}

public:
	armorer(string name) : character(name, stats(3, 5, 0, 12), character_flags::none) {
		make_model("Knight", "none");
		skills = { 
			skill(
				"Stand Guard",
				"Lend defense to an ally",
				no_suit,
				[](card cost, character* c) { return action(
					cost,
					target_ally |
					is_round_buff,
					stats(0, (int)std::ceilf(c->get_stats().defense / 3.0f), 0, 0),
					"stands guard over"
				); }
			),
			skill(
				"First Aid",
				"Performs minor healing on an ally",
				heart,
				[](card cost, character* c) { return action(
					cost, 
					target_ally |
					is_heal,
					stats(0, 0, 0, cost.modifier() + 1),
					"gives first aid to"
				); }
			),
		};
	}
};