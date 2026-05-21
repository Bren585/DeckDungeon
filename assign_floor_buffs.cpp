#include "assign_floor_buff.h"
#include "dungeon_overlay.h"

//#include "BLIB\imgui\imgui.h"
//void assign_floor_buff::update(float elapsed_time) {
//	auto& players = get_players();
//	int choice = -1;
//	if (ImGui::Begin("Select a character to buff")) {
//		for (int i = 0; i < players.size(); i++) {
//			if (ImGui::Button(players[i]->get_name())) { choice = i; }
//		}
//	}
//	ImGui::End();
//
//	if (choice != -1) {
//		players[choice]->buff_for_floor(floor_buff);
//		get_scene()->get_pm().add(
//			new buff_particles(),
//			get_scene()->get_character_model(players[choice]->get_id()).get_pos()
//		);
//		finish();
//	}
//}

void	assign_floor_buff::on_wake	() { get_overlay()->start_listening(this);	}
string	assign_floor_buff::title	() { return "Choose a character to buff";	}

void assign_floor_buff::on_hear	(int choice) {
	auto* player = get_players()[choice];
	player->buff_for_floor(floor_buff); 
	get_scene()->get_pm().add(
		new buff_particles(),
		get_scene()->get_character_model(player->get_id()).get_pos()
	);
	finish();	
}

std::vector<prompt> assign_floor_buff::get_prompts() { 
	std::vector<prompt> out;
	auto& players = get_players();
	for (int i = 0; i < players.size(); i++) { out.emplace_back(players[i]->get_name(), i); }
	return out;
}
