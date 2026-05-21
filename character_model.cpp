#include "character_model.h"
#include "all_behaviors.h"
#include "BLIB\skinned_mesh.h"

character_model::character_model(float3 home, string filename, string alt_texture, bool faces_forward) : home(home), faces_forward(faces_forward) {
	set_model(BLIB::load_fbx(filename, true, BLIB::LH_Y));

	add_animations_fbx(get_model(), "animations/Rig_Medium_General",		0);
	add_animations_fbx(get_model(), "animations/Rig_Medium_MovementBasic",	0);
	add_animations_fbx(get_model(), "animations/Rig_Medium_CombatMelee",	0);
	add_animations_fbx(get_model(), "animations/Rig_Medium_CombatRanged",	0);
	add_animations_fbx(get_model(), "animations/Rig_Medium_Simulation",		0);

	if (alt_texture != "none") {
		string texture = filename;
		texture[0] = tolower(texture[0]);
		texture += "_texture_alt_" + alt_texture + ".png";

		for (auto& mat : get_model()->get_textures()) {
			if (mat.first == 0) continue;
			mat.second.textures[BLIB::texture_map] = std::make_unique<BLIB::material_texture_file>(texture);
			mat.second.force_construct();
			break;
		}
	}

	enqueue<cb::idle>();
}

void character_model::dequeue() {
	in_queue.erase(typeid(*state_queue.front()));
	state_queue.pop();
}

void character_model::next_state() {
	if (state_queue.size() > 0) {
		dequeue();
	}
	if (state_queue.size() == 0) {
		enqueue<cb::idle>();
	}
}

void character_model::update(float elapsed_time) {
	if (get_model() == nullptr) return;
	BLIB::full::object::update(elapsed_time);
	if (state_queue.size() == 0) { next_state(); }
	else {
		state_queue.front()->update(elapsed_time);
		if (state_queue.front()->finished()) { next_state(); }
	}
}

float2 character_model::get_screen_pos(BLIB::camera* cam) {
	matrix VP = cam->get_view_projection();
	float4 clip = mul(float4{ get_pos(), 1 }, VP);
	float2 ndc = clip.xy() / clip.w;
	float2 out = BLIB::window::size() * 0.5f;
	out.x *= (ndc.x + 1);
	out.y *= (ndc.y + 1);
	return out;
}