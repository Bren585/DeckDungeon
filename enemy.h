#pragma once
#include "character.h"
class enemy : public character {
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

public:
	enemy(string name, string model, stats stats) : character(name, stats, character_flags::mindless) {
		make_model(model, "none");
		skills = {

		};
	}
};