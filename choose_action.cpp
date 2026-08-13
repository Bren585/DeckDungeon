#include "choose_action.h"
#include "dungeon_overlay.h"
#include "combat.h"

const string labels[3] = {
	"Choose an action",
	"Choose a reaction",
	"Pay the cost",
};

void		choose_action::update		(float elapsed_time	) {}
void		choose_action::on_wake		(					) { get_overlay()->start_listening(this); }
void		choose_action::on_stop		(					) { get_overlay()->stop_listening(); }

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