#pragma once
#include "BLIB\string.h"

enum character_class {
	class_armorer,
	class_gambler,
	class_mage,
	class_monk,
	class_priest,
	class_shaman,
	class_summoner,
	class_swordsman,
	class_count
};

struct character_data {
	character_class type;
	string name;

	character_data(character_class t, string s) : type(t), name(s) {}
};