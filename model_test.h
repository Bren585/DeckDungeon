#pragma once
#include "BLIB/scene.h"
#include "BLIB/camera_rig.h"
#include "character_model.h"

class model_test : public BLIB::full::scene {
private:
	//BLIB::full::object* test_obj = nullptr;
	character_model* test_obj = nullptr;
	BLIB::full::object backdrop;

	BLIB::perspective_camera cam;

	char buffer[256] = {};

	// Strictly Required for function
	void init() override;
	void update(float elapsed_time) override;
	void draw(BLIB::render_settings) const override;

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}
	void draw_transparent() const override {}

	// Optional
	void kill() override {}
	void on_wake() override {}
	void on_sleep() override {}

public:
	model_test() {}
	~model_test() { if (test_obj) delete test_obj; }
};