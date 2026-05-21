#pragma once
#include "BLIB/scene.h"
#include "card_object.h"
#include "choice_listener.h"

class dungeon_overlay : public BLIB::flat::scene {
private:
	mutable BLIB::task_id dm_id = 0;
	BLIB::canvas overlay;

	BLIB::canvas log_canvas;
	mutable size_t last_log_count = 0;

	BLIB::canvas info_canvas;
	mutable std::vector<card_object> card_sprites;

	BLIB::canvas choice_canvas;
	int on_deck				= no_show;
	int temp_on_deck		= no_show;
	mutable int was_on_deck	= no_show;
	mutable bool force_info	= false;
	std::vector<card_object*> visible_cards;
	std::vector<prompt> choice_prompts;
	mutable std::vector<BLIB::canvas> prompt_buttons;
	choice_listener* listener = nullptr;

	string tooltip;

	// Strictly Required for function
	void init() override;
	void update(float elapsed_time) override;
	void draw(BLIB::render_settings) const override;

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override {}
	void on_wake() override {}
	void on_sleep() override {}

	void render_log() const;

	void render_info() const;

	void update_choice(float elapsed_time);
	void render_choice() const;

	class dungeon_master* get_dm() const;

public:
	dungeon_overlay() : overlay(BLIB::window::size()) {}
	~dungeon_overlay() {}

	void show_info(int id) { on_deck = id; }
	void clear_info() { on_deck = no_show; }

	void start_listening(choice_listener* l);
	void stop_listening() { listener = nullptr; choice_prompts.clear(); prompt_buttons.clear(); if (on_deck != no_show) { on_deck = no_show; } }

	void stop() { finish(); }
};

inline dungeon_overlay* get_overlay() { return static_cast<dungeon_overlay*>(BLIB::manager::get_scene(BLIB::manager::find_first_of_type<dungeon_overlay>())); }