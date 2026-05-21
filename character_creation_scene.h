#pragma once
#include "BLIB/scene.h"
#include "BLIB/camera.h"
#include "character_data.h"


class character_creation_scene : public BLIB::full::scene {
private:
	BLIB::task_id main_scene_id;

	character_class selected_class = class_armorer;

	std::vector<BLIB::full::object> class_models;

	string name;
	canvas textbox;
	bool typing = false;

	std::vector<BLIB::flat::object> buttons;

	BLIB::perspective_camera cam;

	BLIB::flat::object confirm;

	// Strictly Required for function
	void init() override;
	void update(float elapsed_time) override;
	void draw(BLIB::render_settings) const override;

	// Required, but {} is acceptable
	void idle(float elapsed_time) override;
	void draw_transparent() const override;

	// Optional
	void kill() override {}
	void on_wake() override { name = ""; selected_class = class_armorer; }
	void on_sleep() override {}


public:
	character_creation_scene(BLIB::task_id id) : main_scene_id(id) {}
	~character_creation_scene() {}

	character_data get_character_data();
	void stop() { finish(); }
};