#include "start_dungeon_scene.h"
#include "all_classes.h"
#include "dungeon_master.h"
#include "dungeon_overlay.h"
#include "dungeon_scene.h"

character* create_character_from_data(character_data data) {
	switch (data.type) {
	case class_armorer:		return new armorer(data.name);
	case class_gambler:		return new gambler(data.name);
	case class_mage:		return new mage(data.name);
	case class_monk:		return new monk(data.name);
	case class_priest:		return new priest(data.name);
	case class_shaman:		return new shaman(data.name);
	case class_summoner:	return new summoner(data.name);
	default:				return new swordsman(data.name);
	}
}

start_dungeon_scene::start_dungeon_scene(const std::vector<character_data>& party) : 
	party_data(party) ,
	scene_id(BLIB::manager::add(new dungeon_scene))
{}

void start_dungeon_scene::init() {
	for (auto& data : party_data) { players.push_back(create_character_from_data(data)); }

	//players.push_back(new shaman("Shaman"));
	//players.push_back(new monk("Monk"));
	//players.push_back(new priest("Priest"));
	//players.push_back(new swordsman("Swordsman"));
	//players.push_back(new mage("Mage"));
	//players.push_back(new summoner("Summoner"));
	//
	//players.push_back(new character("test", stats(100, -100, -100, 1), none));
	//players.back()->make_model("Knight", "none");

	for (auto& player : players) {
		player->get_deck() = make_deck();
		player->get_deck().shuffle();
	}
}

void start_dungeon_scene::update(float elapsed_time) {
	BLIB::manager::add(new dungeon_master(players));
	BLIB::manager::add_and_stage(new dungeon_overlay, 0, BLIB::transition::fade, 0.5f);
	BLIB::manager::stage(scene_id, 1, BLIB::transition::fade, 2.0f);
	finish();
}