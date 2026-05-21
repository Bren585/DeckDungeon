#include "title_scene.h"
#include "log.h"
#include "mouse_collider.h"
#include "character_creation_scene.h"
#include "start_dungeon_scene.h"

constexpr float add_scale = 0.5f;
constexpr float start_scale = 2.0f;

void title_scene::init() {
	const float padding			= BLIB::window::size().x / 16.0f;
	const float quarter_padding	= padding * 0.25f;
	const float font_height		= BLIB::text::height();

	const float left_edge		= BLIB::window::size().x * 0.5f + padding;
	const float right_edge		= BLIB::window::size().x - padding;

	add_button.make_dummy(GREEN);
	add_button.pivot = C_TL;
	add_button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, add_button.get_size() * 0.5f));
	add_button.set_size(float2{BLIB::text::width("Add Character"), font_height} * add_scale + float2(quarter_padding));
	add_button.get_collider()->set_off(add_button.pivot * add_button.get_size() * -0.5f);

	start_button.make_dummy(WHITE);
	start_button.pos = { left_edge, padding };
	start_button.pivot = C_BL;
	start_button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, start_button.get_size() * 0.5f));
	start_button.set_size({ right_edge - left_edge, font_height * start_scale + padding });
	start_button.get_collider()->set_off(start_button.pivot * start_button.get_size() * -0.5f);

	delete_button.load_sprite("ui/x.png");
	delete_button.pivot = C_TR;
	delete_button.pos.x = right_edge;
	delete_button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, delete_button.get_size() * 0.5f));
	delete_button.set_size(float2{ font_height });
	delete_button.get_collider()->set_off(delete_button.pivot * delete_button.get_size() * -0.5f);

	//string enemy_files[4] = {
	//	"Skeleton_Rogue",
	//	"Skeleton_Mage",
	//	"Skeleton_Minion",
	//	"Skeleton_Warrior"
	//};
	//BLIB::full::object temp;
	//for (int i = 0; i < 4; i++) {
	//	temp.set_model(BLIB::load_fbx(enemy_files[i], true, BLIB::LH_Y));
	//
	//	add_animations_fbx(temp.get_model(), "animations/Rig_Medium_General", 0);
	//	add_animations_fbx(temp.get_model(), "animations/Rig_Medium_MovementBasic", 0);
	//	add_animations_fbx(temp.get_model(), "animations/Rig_Medium_CombatMelee", 0);
	//	add_animations_fbx(temp.get_model(), "animations/Rig_Medium_CombatRanged", 0);
	//	add_animations_fbx(temp.get_model(), "animations/Rig_Medium_Simulation", 0);
	//}
}

void title_scene::kill() {}

void title_scene::update(float elapsed_time) { 
	auto* mouse = get_mouse_collider();
	bool click = BLIB::input::trigger(key::LClick);

	const float padding			= BLIB::window::size().x / 16.0f;
	const float quarter_padding = padding * 0.25f;
	const float font_height		= BLIB::text::height();

	const float top_edge		= BLIB::window::size().y - padding;
	const float left_edge		= BLIB::window::size().x * 0.5f + padding;
	const float right_edge		= BLIB::window::size().x - padding;


	float2 pos = { left_edge, top_edge };
	pos.y -= font_height + quarter_padding;

	for (auto it = party_data.begin(); it != party_data.end();) {
		delete_button.pos.y = pos.y;
		delete_button.update(0);
		pos.y -= font_height * add_scale + quarter_padding;
		if (click) {
			if (BLIB::collision::check(mouse, &delete_button)) { 
				BLIB::audio::play("click");
				it = party_data.erase(it); 
				continue; 
			}
		}
		it++;
	}

	if (party_data.size() < 4) {
		add_button.pos = pos;
		add_button.update(0);
		if (BLIB::collision::check(mouse, &add_button)) {
			if (click) {
				if (!cc_id) { cc_id = BLIB::manager::add(new character_creation_scene(get_id())); }
				else { BLIB::manager::get_task(cc_id)->wake(); }
				BLIB::manager::stage(cc_id, 0, BLIB::transition::fade, 0.5f);
				BLIB::audio::play("click");
			}
			else add_button.tint = WHITE;
		}
		else add_button.tint = float3{ 0.8f };
	}

	start_button.update(0);
	if (party_data.size() > 0) {
		if (BLIB::collision::check(mouse, &start_button)) {
			if (click) {
				if (cc_id) {
					static_cast<character_creation_scene*>(BLIB::manager::get_scene(cc_id))->stop();
					cc_id = 0;
				}
				BLIB::manager::add_and_stage(new start_dungeon_scene(party_data), BLIB::manager::unstage(get_id()), BLIB::transition::fade, 0.5f);
				BLIB::audio::play("click");
				BLIB::audio::stop(bgm_id, 1.0f);
				bgm_id = BLIB::audio::unset;
			}
			else start_button.tint = color(0.5f, 1.0f, 0.5f); // hover
		}
		else start_button.tint = color(0.8f, 1.0f, 0.8f); // no hover, active
	}
	else start_button.tint = color(0.5f, 0.5f, 0.5f); // inactive
}

void title_scene::idle(float elapsed_time) {

}

void title_scene::on_wake() { 
	if (cc_id) {
		party_data.push_back(static_cast<character_creation_scene*>(BLIB::manager::get_scene(cc_id))->get_character_data());
	}
	if (bgm_id == BLIB::audio::unset) {
		bgm_id = BLIB::audio::play("title", 1.0f, true);
	}
}

void title_scene::draw(BLIB::render_settings rs) const {
	const float padding				= BLIB::window::size().x / 16.0f;
	const float quarter_padding		= padding * 0.25f;
	const float sixteenth_padding	= padding * 0.0625f;
	const float font_height			= BLIB::text::height();

	const float top_edge			= BLIB::window::size().y - padding;
	const float left_edge			= BLIB::window::size().x * 0.5f + padding;
	const float right_edge			= BLIB::window::size().x - padding;

	type("Deck Dungeon", get_size().oy() + C_BR * padding, float2{2}, FONT_DEFAULT, WHITE, C_TL);

	float2 pos = { left_edge, top_edge };
	pos.y -= type("Party:", pos, float2{ 1 }, FONT_DEFAULT, WHITE, C_TL) + quarter_padding;

	for (auto& data : party_data) {
		delete_button.pos.y = pos.y;
		delete_button.render(rs);
		pos.y -= type(data.name, pos, float2{ add_scale }, FONT_DEFAULT, WHITE, C_TL) + quarter_padding;
		delete_button.update(0);
		//delete_button.peek_collider()->render_debug(rs);
	}

	if (party_data.size() < 4) {
		add_button.render();
		type("Add Character", pos + C_BR * sixteenth_padding, float2(add_scale), FONT_DEFAULT, WHITE, C_TL);
		//add_button.peek_collider()->render_debug(rs);
	}

	start_button.render();
	type("Enter Dungeon", start_button.pos + C_TR * quarter_padding, float2(start_scale), FONT_DEFAULT, BLACK, C_BL);
	//start_button.peek_collider()->render_debug(rs);

	//type(report() == active ? "active" : "inactive", float2{ padding }, float2{ 1 });
	//type(timer, float2{ padding, padding * 2 }, float2{ 1 });
	//
	//get_mouse_collider()->peek_collider()->render_debug(rs);
}

