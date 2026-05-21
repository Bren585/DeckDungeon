#pragma once
struct stats {
	int attack = 0;
	int defense = 0;
	int luck = 0;
	int health = 0;

	stats(int a = 0, int d = 0, int l = 0, int h = 0) : attack(a), defense(d), luck(l), health(h) {}
	stats(const stats& o) = default;

	stats operator- () const { return stats(-attack, -defense, -luck, -health); }
	stats operator+	(const stats& o) const { return stats(attack + o.attack, defense + o.defense, luck + o.luck, health + o.health); }
	stats operator- (const stats& o) const { return operator+(-o); }

	stats operator+=(const stats& o) { *this = operator+(o); return *this; }
	stats operator-=(const stats& o) { return operator+=(-o); }

	stats keep_highest(const stats& o) { 
		attack	= (attack	< o.attack	? o.attack	: attack	);
		defense = (defense	< o.defense ? o.defense : defense	);
		luck	= (luck		< o.luck	? o.luck	: luck		);
		health	= (health	< o.health	? o.health	: health	);
		return *this;
	}

	int& operator[](int i) {
		switch (i) {
		case 0: return attack;
		case 1: return defense;
		case 2: return luck;
		case 3: return health;
		default:
			assert(false);
			return attack;
		}
	}
};