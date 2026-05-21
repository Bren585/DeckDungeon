#pragma once
#include "character.h"
class summoner : public character {
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

	void start_floor() override { flags += cannot_attack; flags += cannot_target; }

public:
	summoner(string name) : character(name, stats(0, 0, 1, 9), character_flags::summons) {
		make_model("Rogue", "C");
		skills = {
			skill(
				"Circle Enhancement",
				"Enhances your thrall with your own stats",
				any_suit,
				[](card cost, character* c) {
					stats base = c->get_stats();
					stats buff;
					switch (cost.suit()) {
					case spade:		buff.attack		= base.attack	/ 4 + cost.modifier();		break;
					case heart:		buff.health		= base.health	/ 8 + cost.modifier();		break;
					case club:		buff.defense	= base.defense	/ 4 + cost.modifier();		break;
					case diamond:	buff.luck		= (base.luck	/ 4 + cost.modifier()) / 2;	break;
					}

					return action(
						cost,
						is_floor_buff		|
						cannot_be_buffed	|
						target_ally			|
						target_thrall_only,
						buff,
						"enhances"
					);
				}
			)
		};
	}
};

class thrall : public character {
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

	summoner* parent;

	void on_death() override { parent->remove_flag(cannot_attack); parent->remove_flag(cannot_target); }
	void award_exp(int x) override { parent->award_exp(x); }

public:
	thrall(character* parent) : 
		parent(static_cast<summoner*>(parent)),
		character(
			string(parent->get_name(), "'s thrall"),
			parent->get_stats(), 
			character_flags(is_thrall | mindless)
		)
	{
		make_model("Knight", "C");
		skills = {};
		grow_stats({ 0, 0, 0, get_health() });
	}

	summoner* get_parent() { return parent; }
};