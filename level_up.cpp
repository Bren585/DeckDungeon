#include "level_up.h"
#include "dungeon_overlay.h"

level_up::level_up(character* p) : player(p) { levels = p->redeem_exp(); }

void	level_up::on_wake	() { get_overlay()->start_listening(this);	}
void	level_up::on_stop	() { get_overlay()->stop_listening(); }
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