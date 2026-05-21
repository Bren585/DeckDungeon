#pragma once
#include "cards.h"

constexpr int no_show = -1;

struct prompt {
	string name;
	string desc;
	int choice;

	prompt(string n, int choice, string d = "") : name(n), desc(d), choice(choice) {}
};

class choice_listener {
public:
	virtual void on_hear(int choice) = 0;

	virtual string				title		() = 0;
	virtual int					show_info	() { return no_show; }
	virtual bool				allow_cards	() { return false; }
	virtual bool				allow_deck	() { return false; }
	virtual card_suit			allow_suit	() { return any_suit; }
	virtual std::vector<prompt> get_prompts	() { return {}; }
};