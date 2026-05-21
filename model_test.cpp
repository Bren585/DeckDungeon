#include "model_test.h"
#include "BLIB\skinned_mesh.h"

#include "log.h"
#include "BLIB\imgui\imgui.h"
#include "all_behaviors.h"

void model_test::init() {
	test_obj = new character_model({0, 0, 0}, "Skeleton_Mage");
	static_cast<character_model*>(test_obj)->start_behavior<cb::idle>();

	//test_obj = new BLIB::full::object;
	//test_obj->set_model(BLIB::load_fbx("monster/Snake", true, BLIB::LH_Y));
	//test_obj->set_scl(float3{ 0.01f });

	backdrop.set_model(BLIB::load_fbx("dungeon", false, BLIB::LH_Y));
	backdrop.set_qtn(quaternion::face_to({ 0, 0, 1 }, { -1, 0, 0 }));
	float3 size = backdrop.get_size();
	backdrop.add_pos(float3{ size.x * 2, 0, size.z * 2 });
	backdrop.set_scl(float3{ 0.01f });

	for (auto& anim : test_obj->get_model()->get_animations()) {
		event_log::record(anim.second.name);
	}

	cam.set_focus({ 0, 0.5f, 0 });
	cam.set_eye({ 0, 1, 10 });
	//cam = BLIB::perspective_camera(BLIB::window::size(), float3{ 32, 24, -4 }, float3{ 8, 0, 8 }, 0.5235988f, float2(0.01f, 100.0f));
	set_camera(&cam);
}

void model_test::update(float elapsed_time) {
	test_obj->update(elapsed_time);
	event_log::imgui();

	if (ImGui::Begin("Behavior")) {
		if (ImGui::Button("idle")) { static_cast<character_model*>(test_obj)->start_behavior<cb::idle>(); }
		if (ImGui::Button("relax")) { static_cast<character_model*>(test_obj)->start_behavior<cb::relax_start>(); }
		if (ImGui::Button("die")) { static_cast<character_model*>(test_obj)->start_behavior<cb::die>(); }
		//if (ImGui::Button("attack")) { static_cast<character_model*>(test_obj)->start_behavior<cb::punch>(); }
		if (ImGui::Button("block")) { static_cast<character_model*>(test_obj)->start_behavior<cb::block>(); }
		if (ImGui::Button("summon")) { static_cast<character_model*>(test_obj)->start_behavior<cb::summon>(); }
		if (ImGui::Button("stop")) { test_obj->get_model()->stop_animation(); }
	}
	ImGui::End();
}

void model_test::draw(BLIB::render_settings rs) const {
	test_obj->render(rs);
	backdrop.render(rs);
}
