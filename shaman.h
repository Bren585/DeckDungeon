#pragma once
#include "character.h"
class shaman : public character {
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

	int souls = 0;

public:
	shaman(string name) : character(name, stats(3, 3, 1, 9), character_flags::spiritual) {
		make_model("Druid", "none");
		skills = {
			skill(
				"Diabolical Curse",
				"Debuffs all enemies with a card and HP",
				any_suit,
				[](card cost, character* c) {
					int value = c->get_health() / 8 + cost.modifier();
					stats buff;
					switch (cost.suit()) {
					case spade:		buff.attack		= -value;		break;
					case heart:		buff.health		= -value;		break;
					case club:		buff.defense	= -value;		break;
					case diamond:	buff.luck		= -value / 2;	break;
					}

					return action(
						cost,
						(cost.suit_is(heart) ? magic_attack : none) |
						cannot_be_buffed |
						target_all_enemies |
						is_round_debuff,
						buff,
						"curses"
					);
				}
			)
		};
	}

	void	gain_souls(int n)	{ souls += n; }
	int		redeem_souls()		{ int exp = 0; while (souls >= 5) { souls -= 5; exp += 2; } return exp; }
};