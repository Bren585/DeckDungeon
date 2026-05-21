#pragma once
#include "BLIB\scene.h"
#include "character_data.h"
#include "BLIB\audio.h"

class title_scene : public BLIB::flat::scene {
private:
	int bgm_id = BLIB::audio::unset;

	BLIB::task_id cc_id = 0;

	BLIB::flat::object bkg;
	BLIB::flat::object start_button;
	BLIB::flat::object add_button;
	mutable BLIB::flat::object delete_button;

	std::vector<character_data> party_data;

	// Strictly Required for function
	void init() override;
	void update(float elapsed_time) override;
	void draw(BLIB::render_settings) const override;

	// Required, but {} is acceptable
	void idle(float elapsed_time) override;

	// Optional
	void kill() override;
	void on_wake() override;
	void on_sleep() override {}

public:
	title_scene() {}
	~title_scene() {}
};