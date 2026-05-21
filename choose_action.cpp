#include "choose_action.h"
#include "dungeon_overlay.h"
#include "combat.h"

const string labels[3] = {
	"Choose an action",
	"Choose a reaction",
	"Pay the cost",
};

#ifdef choose_action_imgui
#include "BLIB\imgui\imgui.h"
#include "dungeon_master.h"
void choose_action::update(float elapsed_time) {
	bool chose = false;
	int choice = 0;
	string label = labels[(type & action_skill) ? 2 : (int)type];

	if (ImGui::Begin(label)) {
		for (int i = 0; i < hand->size(); i++) {
			card c = (*hand)[i];
			if (type & action_skill) {
				int cost_type = 1 << ((int)c.suit() + 1);
				if ((cost_type & type) == 0) continue;
			}
			label = string("Play ", c);
			if (ImGui::Button(label))			{ choice = i;				chose = true; }
		}
		
		if ((type & action_skill) == 0) {
			if (ImGui::Button("Top Deck"))		{ choice = top_deck;		chose = true; }
		}

		if (type == action_any) {
			if (ImGui::Button("Use a Skill"))	{ choice = use_skill;		chose = true; }
		}

		if (type & action_skill) {
			if (ImGui::Button("Cancel"))		{ choice = cancel_skill;	chose = true; }
		}
	}
	ImGui::End();

	if (chose) {
		parent->set_choice(choice);
		finish();
	}
}

void choose_action::init() { hand = &GET_DM(BLIB::manager::find_first_of_type<dungeon_master>())->peek_players()[id]->peek_hand(); }

#else
void		choose_action::update		(float elapsed_time	) {}
void		choose_action::on_wake		(					) { get_overlay()->start_listening(this); }
#endif
void		choose_action::on_hear		(int choice			) { parent->set_choice(choice); finish(); }
string		choose_action::title		(					) { return labels[(type & action_skill) ? 2 : (int)type]; }
int			choose_action::show_info	(					) { return id; }
bool		choose_action::allow_cards	(					) { return true; }
bool		choose_action::allow_deck	(					) { return (type & action_skill) == 0; }

card_suit choose_action::allow_suit	() {
	if (type & action_skill) {
		switch (type) {
		case action_pay_spade:		return spade;
		case action_pay_heart:		return heart;
		case action_pay_club:		return club;
		case action_pay_diamond:	return diamond;
		}
	}
	return any_suit;
}

std::vector<prompt> choose_action::get_prompts() { 
	std::vector<prompt> out_prompts;
	if (type & action_skill)	out_prompts.emplace_back("Cancel", cancel_skill);
	//else						out_prompts.emplace_back("Top Deck", top_deck);
	if (type == action_any)		out_prompts.emplace_back("Use a Skill", use_skill);
	return out_prompts;
}