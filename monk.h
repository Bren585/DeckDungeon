#pragma once
#include "character.h"
class monk : public character {
	action basic_attack(card card) override {
		switch (card.suit()) {
		case spade:		return DEFAULT_ATTACK;
		case club:		return DEFAULT_WHIFF;
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

	void damage	(int d) override { if (d > 0) { flags -= concentrated; } character::damage(d);	}
	void heal	(int h) override { if (h < 0) { flags -= concentrated; } character::heal(h);	}

	void start_floor() override { flags += concentrated; }

public:
	monk(string name) : character(name, stats(2, 5, 1, 12), character_flags::none) {
		make_model("Ranger", "C");
		skills = {
			skill(
				"Prayer of Faith",
				"Buffs all allies with a card and HP",
				any_suit,
				[](card cost, character* c) {
					int value = c->get_health() / 10 + cost.modifier();
					stats buff;
					switch (cost.suit()) {
					case spade:		buff.attack		= value;		break;
					case heart:		buff.health		= value;		break;
					case club:		buff.defense	= value;		break;
					case diamond:	buff.luck		= value / 2;	break;
					}

					return action(
						cost,
						is_round_buff |
						cannot_be_buffed |
						target_all_allies |
						can_target_self,
						buff,
						"prays for"
					);
				}
			)
		};
	}
};