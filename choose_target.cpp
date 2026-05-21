#include "choose_target.h"
#include "combat.h"
#include "dungeon_overlay.h"

//#include "BLIB\imgui\imgui.h"
//void choose_target::update(float elapsed_time) {
//	bool chose = false;
//	int choice = 0;
//	string label;
//
//	if (ImGui::Begin("Choose a target")) {
//		for (int i = 0; i < targets.size(); i++) {
//			if (ImGui::Button(targets[i]->get_name())) { choice = i; chose = true; }
//		}
//	}
//	ImGui::End();
//
//	if (chose) {
//		parent->set_choice(choice);
//		finish();
//	}
//}

void	choose_target::on_wake	(			) { get_overlay()->start_listening(this);	}
void	choose_target::on_hear	(int choice	) { parent->set_choice(choice); finish();	}
string	choose_target::title	(			) { return "Choose a Target";				}

std::vector<prompt> choose_target::get_prompts() {
	std::vector<prompt> out;
	for (int i = 0; i < targets.size(); i++) { out.emplace_back(targets[i]->get_name(), i); }
	return out;
}
