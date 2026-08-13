#include "choose_target.h"
#include "combat.h"
#include "dungeon_overlay.h"

void	choose_target::on_wake	(			) { get_overlay()->start_listening(this);	}
void	choose_target::on_stop	(			) { get_overlay()->stop_listening();		}
void	choose_target::on_hear	(int choice	) { parent->set_choice(choice); finish();	}
string	choose_target::title	(			) { return "Choose a Target";				}

std::vector<prompt> choose_target::get_prompts() {
	std::vector<prompt> out;
	for (int i = 0; i < targets.size(); i++) { out.emplace_back(targets[i]->get_name(), i); }
	return out;
}
