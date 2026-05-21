#include "choose_skill.h"
#include "combat.h"
#include "dungeon_overlay.h"

//#include "BLIB\imgui\imgui.h"
//void choose_skill::update(float elapsed_time) {
//	bool chose = false;
//	int choice = 0;
//	string label;
//
//	if (ImGui::Begin("Choose a skill")) {
//		for (int i = 0; i < skills.size(); i++) {
//			if (ImGui::Button(skills[i].name)) { choice = i; chose = true; }
//			ImGui::SameLine();
//			ImGui::Text(skills[i].description);
//		}
//		if (ImGui::Button("Cancel")) { choice = cancel_skill; chose = true; }
//	}
//	ImGui::End();
//
//	if (chose) {
//		parent->set_choice(choice);
//		finish();
//	}
//}

void	choose_skill::on_wake	(					) { get_overlay()->start_listening(this);	}
void	choose_skill::update	(float elapsed_time	) {											}
void	choose_skill::on_hear	(int choice			) { parent->set_choice(choice); finish();	}
string	choose_skill::title		(					) { return "Choose a Skill";				}

std::vector<prompt> choose_skill::get_prompts() { 
	std::vector<prompt> out;
	for (int i = 0; i < skills.size(); i++) { out.emplace_back(skills[i].name, i, skills[i].description); }
	out.emplace_back("Cancel", cancel_skill);
	return out;
}
