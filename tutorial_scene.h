#pragma once
#include "BLIB/scene.h"
class tutorial_scene : public BLIB::flat::scene {
private:
	BLIB::flat::object bkg;

	// Strictly Required for function
	void init() override;
	void update(float elapsed_time) override;
	void draw(BLIB::render_settings rs) const override { bkg.render(rs); }

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void uninit() override {}
	void on_wake() override {}
	void on_sleep() override {}

public:
	tutorial_scene() {}
	~tutorial_scene() {}
};