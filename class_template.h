#pragma once
#include "character.h"
class template_class : public character {
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
	template_class(string name) : character(name, stats(0, 0, 0, 0), character_flags::none) {
		make_model("model_name", "none");
		skills = {

		};
	}
};