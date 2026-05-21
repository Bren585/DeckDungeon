#pragma once
#include "BLIB\status.h"
#include "choice_listener.h"

//#define choose_action_imgui

class combat;
class pile;

enum action_type {
	action_any = 0,
	action_reaction = 1,

	action_pay_spade	= 1 << 1,
	action_pay_heart	= 1 << 2,
	action_pay_club		= 1 << 3,
	action_pay_diamond	= 1 << 4,

	action_pay_any		= action_pay_spade | action_pay_heart | action_pay_club | action_pay_diamond,
	action_skill		= action_pay_any,
};

class choose_action : public BLIB::status, public choice_listener {
private:

	combat* parent;
#ifdef choose_action_imgui
	const pile* hand = nullptr;
#endif
	int id;
	action_type type;

	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override;

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override {}
	void on_wake() override;
	void on_sleep() override {}

public:
	choose_action(combat* ptr, int id/*const pile* hand*/, action_type type) : parent(ptr), id(id)/*hand(hand)*/, type(type) {}
	~choose_action() {}

	void on_hear(int choice) override;

	string				title		() override;
	int					show_info	() override;
	bool				allow_cards	() override;
	bool				allow_deck	() override;
	card_suit			allow_suit	() override;
	std::vector<prompt> get_prompts	() override;

};