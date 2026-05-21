#include "revive_players.h"
#include "character.h"
#include "dungeon_overlay.h"

revive_players::revive_players(character* shaman, std::vector<character*>& dead_players) :
	shaman(shaman),
	dead_players(dead_players)/*,
	sacrifice_pool(shaman->get_stats().attack) */
{
	//sacrifice.resize(dead_players.size(), 0);
}

//int sacrifice_to_health(int sacrifice) {
//	return sacrifice > 5 ? 3 + (sacrifice - 5 * 6) : 0;
//}

//#include "BLIB\imgui\imgui.h"
//void revive_players::update(float elapsed_time) {
//	if (dead_players.size() == 0 || shaman->get_stats().attack < 5) {
//		finish();
//		return;
//	}
//
//	string label;
//
//	if (ImGui::Begin("Revive dead players")) {
//
//		label = string("Reviving a dead player requires ", shaman->get_name(), " to sacrifice 5 attack");
//		ImGui::Text(label);
//		ImGui::Text("A revived player has 3 health. Each additional attack sacrificed will heal 6 health.");
//
//		label = string(shaman->get_name(), " has ", sacrifice_pool, " attack");
//		ImGui::Text(label);
//		ImGui::Text("WARNING: Dead players who are not revived now are dead FOREVER.");
//
//		for (int i = 0; i < sacrifice.size(); i++) {
//			auto& player		= dead_players[i];
//			auto& sacrificed	= sacrifice[i];
//			int health = sacrifice_to_health(sacrificed);
//
//			label = string(player->get_name(), " ", health, " / ", player->get_stats().health);
//			ImGui::Text(label);
//			ImGui::SameLine();
//
//			if (sacrificed == 0) {
//				label = string("+", i);
//				if (ImGui::Button(label) && sacrifice_pool > 5) { sacrifice_pool -= 5; sacrificed = 5; }
//			}
//			else {
//				label = string("-", i);
//				if (ImGui::Button(label)) { 
//					if (sacrificed == 5) { sacrificed = 0; sacrifice_pool += 5; }
//					else { sacrificed--; sacrifice_pool++; }
//				}
//				ImGui::SameLine();
//				label = string("+", i);
//				if (ImGui::Button(label) && sacrifice_pool > 0) { sacrifice_pool--; sacrificed++; }
//			}
//		}
//
//		if (ImGui::Button("Confirm")) {
//			for (int i = 0; i < sacrifice.size(); i++) {
//				dead_players[i]->heal(sacrifice_to_health(sacrifice[i]));
//				shaman->grow_stats(stats(-sacrifice[i], 0, 0, 0));
//				get_scene()->get_character_model(dead_players[i]->get_id()).start_behavior<cb::relax_end>("Lie_");
//			}
//			finish();
//		}
//	}
//	ImGui::End();
//}

void	revive_players::on_wake		() { get_overlay()->start_listening(this);	}
int		revive_players::show_info	() { return shaman->get_id();				}

string revive_players::title() {
	switch (sacrifice_state) {
	case sacrifice_select:		
		return string(shaman->get_name(), ", sacrifice 5 ATK to revive a player");
	case sacrifice_complete:	
		auto* p = dead_players[revive_index];
		return string(p->get_name(), " will have ", p->get_health(), " HP");
	}
}

void revive_players::on_hear(int choice) {
	switch (sacrifice_state) {
	case sacrifice_select:
		if (choice == -1) { finish(); return; }
		revive_index = choice;
		get_scene()->get_character_model(dead_players[revive_index]->get_id()).start_behavior<cb::relax_end>("Lie_");
		dead_players[revive_index]->heal(3);
		shaman->grow_stats({ -5, 0, 0, 0 });
		if (shaman->get_stats().attack < 1) { finish(); return; }
		sacrifice_state = sacrifice_complete;
		sleep();
		break;
	case sacrifice_complete:
		if (choice) {
			dead_players[revive_index]->heal(6);
			shaman->grow_stats({ -1, 0, 0, 0 });
			if (shaman->get_stats().attack < 1) { finish(); return; }
			sleep();
		} 
		else {
			dead_players.erase(dead_players.begin() + revive_index);
			if (shaman->get_stats().attack < 5 || dead_players.size() < 0) { finish(); return; }
			sacrifice_state = sacrifice_select;
			sleep();
		}
		break;
	}

}

std::vector<prompt> revive_players::get_prompts() {
	switch (sacrifice_state) {
	case sacrifice_select:
	{
		std::vector<prompt> out;
		for (int i = 0; i < dead_players.size(); i++) { out.emplace_back(dead_players[i]->get_name(), i); }
		out.emplace_back("Let them die", -1);
		return out;
	}
	case sacrifice_complete:
		return {
			{"Sacrifice 1 ATK to heal 6 HP", 1},
			{"Stop healing", 0}
		};
	}
	
}