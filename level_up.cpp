#include "level_up.h"
#include "dungeon_overlay.h"

level_up::level_up(character* p) : player(p) { levels = p->redeem_exp(); }

//#include "BLIB\imgui\imgui.h"
//void level_up::update(float elapsed_time) {
//
//	string label(player->get_name(), ", level up!");
//	if (ImGui::Begin(label)) {
//		label = string(levels, " levels available");
//		ImGui::Text(label);
//
//		stats base = player->get_stats();
//
//		label = string("Health: ", player->get_health(), "/", base.health, "(", to_award.health * 3, ")");
//		ImGui::Text(label);
//		ImGui::SameLine();
//		if (ImGui::Button("-h") && to_award.health	> 0) { to_award.health--;	levels++;			}
//		ImGui::SameLine();
//		if (ImGui::Button("+h") && levels			> 0) { levels--;			to_award.health++;	}
//		
//		label = string("Attack: ", base.attack, " (", to_award.attack, ")");
//		ImGui::Text(label);
//		ImGui::SameLine();
//		if (ImGui::Button("-a") && to_award.attack	> 0) { to_award.attack--;	levels++;			}
//		ImGui::SameLine();
//		if (ImGui::Button("+a") && levels			> 0) { levels--;			to_award.attack++;	}
//		
//		label = string("Defense: ", base.defense, " (", to_award.defense, ")");
//		ImGui::Text(label);
//		ImGui::SameLine();
//		if (ImGui::Button("-d") && to_award.defense	> 0) { to_award.defense--;	levels++;			}
//		ImGui::SameLine();
//		if (ImGui::Button("+d") && levels			> 0) { levels--;			to_award.defense++;	}
//		
//		label = string("Luck: ", base.luck, " (", to_award.luck, ")");
//		ImGui::Text(label);
//		ImGui::SameLine();
//		if (ImGui::Button("-l") && to_award.luck		> 0) { to_award.luck--;		levels++;		}
//		ImGui::SameLine();
//		if (ImGui::Button("+l") && levels			> 0) { levels--;			to_award.luck++;	}
//
//		if (levels == 0) {
//			if (ImGui::Button("Confirm")) { 
//				to_award.health *= 3;
//				player->grow_stats(to_award);
//				finish(); 
//			}
//		}
//
//	}
//	ImGui::End();
//}

void	level_up::on_wake	() { get_overlay()->start_listening(this);	}
string	level_up::title		() { return string(player->get_name(), " you have ", levels, " level", (levels == 1 ? "": "s"), "!"); }

void level_up::on_hear(int choice) {
	stats award;
	award[choice] += choice == 3 ? 3 : 1;
	player->grow_stats(award);
	levels--;

	if (levels > 0) { sleep(); }
	else finish();
}

std::vector<prompt> level_up::get_prompts() {
	return {
		{"+3 HP", 3},
		{"+1 ATK", 0},
		{"+1 DEF", 1},
		{"+1 LCK", 2}
	};
}