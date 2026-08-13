#include "choose_skill.h"
#include "combat.h"
#include "dungeon_overlay.h"

void	choose_skill::on_wake	(					) { get_overlay()->start_listening(this);	}
void	choose_skill::on_stop	(					) { get_overlay()->stop_listening();		}
void	choose_skill::update	(float elapsed_time	) {											}
void	choose_skill::on_hear	(int choice			) { parent->set_choice(choice); finish();	}
string	choose_skill::title		(					) { return "Choose a Skill";				}

std::vector<prompt> choose_skill::get_prompts() { 
	std::vector<prompt> out;
	for (int i = 0; i < skills.size(); i++) { out.emplace_back(skills[i].name, i, skills[i].description); }
	out.emplace_back("Cancel", cancel_skill);
	return out;
}
