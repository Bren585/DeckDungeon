#pragma once
#include "character.h"
class priest : public character {
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

	void end_round() override { flags -= cannot_target; character::end_round(); }

public:
	priest(string name) : character(name, stats(4, 2, 4, 6), character_flags::none) {
		make_model("Mage", "A");
		skills = {
			skill(
				"Sacred Stowaway",
				"Hides behind an ally, lending them defense and luck",
				no_suit,
				[](card cost, character* c) { 
					c->set_flag(cannot_target);
					stats base = c->get_stats();
					return action(
						cost,
						target_ally |
						is_round_buff,
						stats(0, base.defense, base.luck, 0),
						"hides behind"
					); 
				}
			),
			skill(
				"Healing Prayer",
				"Heals the whole party a little bit",
				any_suit,
				[](card cost, character* c) {
					int numerator = 1;
					int denominator = 1;
					switch (cost.suit()) {
					case spade:						denominator = 3;	break; // 1/3
					case heart:											break; // 1
					case club:						denominator = 2;	break; // 1/2
					case diamond:	numerator = 2;	denominator = 3;	break; // 2/3
					}

					int heal = (c->get_stats().luck * numerator) / denominator + cost.modifier();

					return action(
						cost,
						target_all_allies | 
						is_heal,
						stats(0, 0, 0, heal),
						"prays for the health of"
					);
				}
			),
			skill(
				"Urgent Care",
				"Heals the one ally a lot",
				any_suit,
				[](card cost, character* c) {
					int numerator = 2;
					int denominator = 1;
					switch (cost.suit()) {
					case spade:						denominator = 3;	break; // 2/3
					case heart:											break; // 2
					case club:						denominator = 2;	break; // 1
					case diamond:	numerator = 3;	denominator = 2;	break; // 3/2
					}

					int heal = (c->get_stats().luck * numerator) / denominator + cost.modifier();

					return action(
						cost,
						target_ally | 
						is_heal,
						stats(0, 0, 0, heal),
						"gives urgent care to"
					);
				}
			),
			skill(
				"Defibrillator",
				"Attempts to revive an ally",
				any_suit,
				[](card cost, character* c) {
					int numerator = 2;
					int denominator = 1;
					switch (cost.suit()) {
					case spade:						denominator = 3;	break; // 2/3
					case heart:											break; // 2
					case club:						denominator = 2;	break; // 1
					case diamond:	numerator = 3;	denominator = 2;	break; // 3/2
					}

					int heal = (c->get_stats().luck * numerator) / denominator + cost.modifier();
					
					int rng = rand() % (c->get_stats().luck * 2);

					if (heal < rng) { heal = 0; }
					else { heal = c->get_stats().health / 2; }

					return action(
						cost,
						target_ally |
						target_dead_allies |
						is_heal,
						stats(0, 0, 0, heal),
						"attemps to resuscitate"
					);
				}
			)
		};
	}
};