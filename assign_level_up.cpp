#include "assign_level_up.h"
#include "dungeon_overlay.h"
#include "dungeon_scene.h"

//#include "BLIB\imgui\imgui.h"
//void assign_level_up::update(float elapsed_time) {
//	string label("Assign extra levels");
//	if (ImGui::Begin(label)) {
//		label = string(to_assign, " levels available");
//		ImGui::Text(label);
//
//		for (int i = 0; i < assigned.size(); i++) {
//			auto& player = get_players()[i];
//			auto& levels = assigned[i];
//
//			label = player->get_name();
//			if (player->dead()) label += " RIP";
//			else label += string(" (", levels, ")");
//			ImGui::Text(label);
//
//			if (player->dead()) continue;
//
//			label = string("-", i);
//			ImGui::SameLine();
//			if (ImGui::Button(label) && levels		> 0) { levels--;	to_assign++;	}
//			label = string("+", i);
//			ImGui::SameLine();
//			if (ImGui::Button(label) && to_assign	> 0) { to_assign--;	levels++;		}
//		}
//
//		if (to_assign == 0) {
//			if (ImGui::Button("Confirm")) {
//				for (int i = 0; i < assigned.size(); i++) {
//					get_players()[i]->award_exp(assigned[i]);
//				}
//				finish();
//			}
//		}
//	}
//	ImGui::End();
//
//}

void	assign_level_up::on_wake	() { get_overlay()->start_listening(this);	}
string	assign_level_up::title		() { return string("Award ", to_assign, " bonus levels"); }

void assign_level_up::on_hear (int choice) {
	auto* player = get_players()[choice];
	player->award_exp(1); 
	get_scene()->get_character_model(player->get_id()).start_behavior<cb::celebrate>();
	to_assign--;
	if (to_assign > 0) { sleep(); }
	else finish();	
}

std::vector<prompt> assign_level_up::get_prompts() {
	std::vector<prompt> out;
	auto& players = get_players();
	for (int i = 0; i < players.size(); i++) { out.emplace_back(players[i]->get_name(), i); }
	return out;
}
