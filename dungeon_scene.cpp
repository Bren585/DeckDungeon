#include "dungeon_scene.h"
//#include "BLIB/imgui/imgui.h"

#include "all_behaviors.h"
#include "character.h"
#include "summoner.h"

#include "dungeon_master.h"

constexpr float character_gap = 4.0f;
constexpr float spawn_z = -8.0f;
constexpr float enemy_spawn_z = 16.0f;

//void imgui_character_window(const character* c) {
//	if (ImGui::Begin(c->get_name())) {
//		const stats base = c->get_stats();
//		const stats buff = c->get_buffs();
//		string text = string("Health: ", c->get_health(), "/", base.health);
//		ImGui::Text(text);
//		text = string("Attack: ", base.attack, " (", buff.attack, ")");
//		ImGui::Text(text);
//		text = string("Defense: ", base.defense, " (", buff.defense, ")");
//		ImGui::Text(text);
//		text = string("Luck: ", base.luck);
//		ImGui::Text(text);
//		ImGui::Text("- hand ----------------------");
//		const pile& hand = c->peek_hand();
//		for (const card& card : hand) {
//			text = card;
//			ImGui::Text(text);
//		}
//
//		int total_cards = int(c->peek_deck().size() + c->peek_hand().size());
//		if (total_cards != 52 && total_cards != 0) ImGui::Text("I'm missing cards!!");
//	}
//	ImGui::End();
//}

void dungeon_scene::init() {
	cam = BLIB::perspective_camera(BLIB::window::size(), float3{ 18.5f, 25, -23.5f }, float3{ 8.5f, 0, 8.5f });
	set_camera(&cam);

	pos = {-140, 80};
	get_scene_lights().set_skylight_direction({ -0.458f, -0.772f, 0.441f });

	backdrop.set_model(BLIB::load_fbx("dungeon", false, BLIB::LH_Y));
	backdrop.set_qtn(quaternion::face_to({ 0, 0, 1 }, { -1, 0, 0 }));
	float3 size = backdrop.get_size();
	backdrop.add_pos(float3{ size.x * 2, 0, size.z * 2 });
	backdrop.set_scl(float3{ 0.01f });

	BLIB::light& spot_light = get_lights().emplace_back();
	spot_light.disable();
	spot_light.set_direction(float3(-0.1f, -1, 0).norm());
	spot_light.set_fade(0.8f);
	spot_light.set_spread(0.3f);
	spot_light.set_position({ 0, 5, 0 });
	spot_light.set_intensity(10);
}

void dungeon_scene::kill() {
	
}

void dungeon_scene::stop() {
	finish();
}

void dungeon_scene::update(float elapsed_time) {
	idle(elapsed_time);

	//auto* dm = GET_DM(BLIB::manager::find_first_of_type<dungeon_master>());
	//for (auto& player : dm->peek_players()) { imgui_character_window(player); }
	//for (auto& enemy : dm->peek_enemies()) { imgui_character_window(enemy); }

}

void dungeon_scene::idle(float elapsed_time) {
	for (auto& it : character_models) { it.second.update(elapsed_time); }
	for (auto& it : enemy_models) { it.second.update(elapsed_time); }

	for (auto it = graveyard.begin(); it != graveyard.end(); ) {
		if (character_models.find(*it) == character_models.end()) { it = graveyard.erase(it); continue; }
		if (!character_models[*it].behavior_scheduled<cb::disappear>()) {
			character_models.erase(*it);
			it = graveyard.erase(it); continue;
		}
		++it;
	}
	pm.update(elapsed_time);
	//event_log::imgui();
}

void dungeon_scene::register_character_model(int id, string filename, string alt_texture, character* character) {
	float3 home;
	if (character->has_flag(mindless)) {
		if (character->has_flag(is_thrall)) {
			const int parent_id = static_cast<thrall*>(character)->get_parent()->get_id();
			home = character_models[parent_id].get_home() + float3{0, 0, character_gap};

			character_models.emplace(id, character_model(home, filename, alt_texture));
			character_models[id].set_pos(home);
			character_models[id].set_qtn(character_models[id].get_forward());
			character_models[id].start_behavior<cb::summon>();
		}
		else {
			const int num_enemies = (int)enemy_models.size();
			home = float3(character_gap * num_enemies, 0, enemy_spawn_z);

			enemy_models.emplace(id, character_model(home, filename, alt_texture, false));
			enemy_models[id].set_pos(home);
			enemy_models[id].set_qtn(enemy_models[id].get_forward());
			enemy_models[id].start_behavior<cb::summon>();
		}
	}
	else {
		const int num_characters = (int)character_models.size();
		home = float3(character_gap * num_characters, 0, 0);
		character_models.emplace(id, character_model(home, filename, alt_texture));
		character_models[id].set_pos(float3{ home.x, 0, spawn_z });
		character_models[id].start_behavior<cb::go_home>(1.0f);
	}
}

void dungeon_scene::spotlight(int id, color c) {
	auto& light = get_lights()[0];
	character_model* character = character_models.find(id) == character_models.end() ? &enemy_models.at(id) : &character_models.at(id);
	light.set_position(light.get_position().oyo() + character->get_home().xoz() + character->get_size().xoz() * 0.25f);
	light.set_tint(c);
	light.enable();
}

void dungeon_scene::clear_spotlight() { get_lights()[0].disable(); }