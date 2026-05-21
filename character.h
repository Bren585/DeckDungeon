#pragma once
#include "actions.h"

class character;
struct skill {
	typedef action (*skill_effect)(card, character*);
	string name;
	string description;
	card_suit cost;
	skill_effect effect;

	skill(string name, string desc, card_suit cost, skill_effect e) : name(name), description(desc), cost(cost), effect(e) {}
	skill() : skill("null", "null", no_suit, [](card cost, character* c) { return action(); }) {}
};

enum character_flags {
	none			= 0,
	is_lucky		= 1 << 0,
	concentrated	= 1 << 1,
	spiritual		= 1 << 2,
	mindless		= 1 << 3,
	cannot_attack	= 1 << 4,
	summons			= 1 << 5,
	cannot_target	= 1 << 6,
	is_thrall		= 1 << 7, 
};

inline character_flags operator+= (character_flags& a, character_flags b) { a = (character_flags)(a | b); return a; }
inline character_flags operator-= (character_flags& a, character_flags b) { a = (character_flags)(a & ~b); return a; }

class character {
private:
	string name;
	int health;
	int exp = 0;

	stats base;
	stats floor_buff;
	stats round_buff;
	stats turn_buff;

	static int id_counter;
	int id;

	pile deck;
	pile hand;

protected:
	std::vector<skill> skills;
	character_flags flags = {};

public:
	character(string name, stats s, character_flags f);
	bool operator==(const character& o) { return id == o.id; }
	bool operator!=(const character& o) { return !operator==(o); }

	void make_model(string model, string alt);

	string get_name() const { return name; }

	bool	alive			() const { return health > 0;	}
	bool	dead			() const { return !alive();		}	
	int		get_health		() const { return health;		}

	virtual void on_death() {}

	virtual void damage	(int d) { health -= d; if (health < 0			) health = 0;			}
	virtual void heal	(int h) { health += h; if (health > base.health	) health = base.health; }

	virtual void award_exp(int amount) { exp += amount; }
	int peek_exp() const { return exp; }
	int redeem_exp() { int redeemed = exp; exp = 0; return redeemed; }

	void grow_stats(stats s) { base += s; health += s.health; }

	stats get_stats			() const { return base; }
	stats get_buffs			() const { return floor_buff + round_buff + turn_buff; }
	stats get_buffed_stats	() const { return get_stats() + get_buffs(); }

	int get_luck_bonus() const {
		int divisor = (flags & is_lucky) ? 2 : 3;
		return (base.luck / divisor) + bool(base.luck % divisor);
	}

	void buff_for_floor	(stats buff)	{ floor_buff += buff; heal(buff.health); }
	void buff_for_round	(stats buff)	{ round_buff += buff; heal(buff.health); }
	void buff_for_turn	(stats buff)	{ turn_buff = turn_buff.keep_highest(buff); }

	virtual void start_floor() {}

	virtual void end_floor() { floor_buff = {}; end_round(); }
	virtual void end_round() { round_buff = {}; end_turn();  }
	virtual void end_turn () { turn_buff  = {}; }

	pile& get_hand() { return hand; }
	pile& get_deck() { return deck; }

	const pile& peek_hand() const { return hand; }
	const pile& peek_deck() const { return deck; }

	auto& get_skills() { return skills; }

	bool has_flag	(character_flags flg) const { return flags & flg; }
	void set_flag	(character_flags flg)		{ flags += flg; }
	void remove_flag(character_flags flg)		{ flags -= flg; }

	int get_id() const { return id; }

#define ACTION(flags, buff, name) action(card, flags, stats ##buff, name)

#define DEFAULT_ATTACK	ACTION(target_enemy | is_combat | attack_bonus, {}, "attacks")
#define DEFAULT_DEFEND	ACTION(defense_bonus, {}, "defends")
#define DEFAULT_WHIFF	ACTION(target_enemy | is_combat, {}, "whiffs")
#define DEFAULT_LUCK	ACTION(can_target_self | act_again | is_turn_buff, (0, 0, card.modifier() + get_luck_bonus(), 0), "hypes up")
#define DEFAULT_HEAL	ACTION(target_enemy | heal_bonus | is_combat, (0, 0, 0, card.modifier()), "heals while approaching")

	virtual action basic_attack(card card) {
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

	virtual action reaction(card card) {
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

};