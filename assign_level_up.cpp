#include "assign_level_up.h"
#include "dungeon_overlay.h"
#include "dungeon_scene.h"

void	assign_level_up::on_wake	() { get_overlay()->start_listening(this);	}
void	assign_level_up::on_stop	() { get_overlay()->stop_listening(); }
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
