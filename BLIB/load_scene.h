#pragma once
#include "manager.h"
#include "entity.h"
#include "scene.h"

/*
	load_scene is used internally by the scene manager to load new scenes before they are staged.

	load_scene_any is an interface that can be used to edit the characteristics of all loading screens.
*/

namespace BLIB {

	class load_scene_any {
	protected:
		inline static color		background_color	= BLACK;
		inline static color		text_color			= WHITE;
		inline static string	background_filename = L"-1";
		inline static string	load_icon_filename	= L"-1";
		inline static string	load_text			= "Loading...";
		inline static string	load_font			= FONT_DEFAULT;
		inline static float		spin_speed			= PI2;

	public:
		// Change the background color of all loading screens.
		static void set_background		(color c)		{ background_color = c; }
		// Load a sprite to use as the background of all loadings screens.
		static void set_background		(string file)	{ background_filename = file; }
		// Load a sprite to use as the spinning load icon  of all loadings screens.
		static void set_load_icon		(string file)	{ load_icon_filename = file; }
		// Set the message in the bottom left  of all loadings screens.
		static void set_text			(string text)	{ load_text = text; }
		// Set the color of the loading screen message.
		static void set_text_color		(color c)		{ text_color = c; }
		// Set the font of the loading screen message.
		static void set_text_font		(string f)		{ load_font = f; }
		// Set the speed of the spin of the loading screen icon.
		static void set_icon_spin_speed	(float f)		{ spin_speed = f;}

	};

	class load_scene : public flat::scene, load_scene_any {
	private:
		task_id scene_id;
		int slot;
		flat::object background;
		flat::object load_icon;

		transition	exit_transition;
		float		exit_duration;

		void on_load() override { 
			if (manager::peek_slot(slot)->get_id() == get_id()) force_wake();
			else												force_sleep();
		}

	public:
		load_scene(task_id id, int slot, transition t = transition::none, float duration = 0) : slot(slot), scene_id(id), exit_transition(t), exit_duration(duration) { canvas::set_background(background_color); }
		
		template<class S, typename... Args>
		load_scene(int slot, transition t = transition::none, float duration = 0, Args&&... args) : slot(slot), scene_id(manager::add(new S(std::forward<Args>(args)...))), exit_transition(t), exit_duration(duration) { canvas::set_background(background_color); }

		inline void init() override {
			if (background_filename != L"-1") {
				background.load_sprite(background_filename);
				background.pivot = C_BL;
				background.set_size(float2{ window::size().x });
			}

			if (load_icon_filename != L"-1") {
				load_icon.load_sprite(load_icon_filename);
				load_icon.pivot = C_CC;
				load_icon.set_size(float2{ 100 });
				load_icon.pos = { window::size().x - 100.0f, 100.0f };
			}
		}

		void update	(float elapsed_time)		override { if (manager::peek_task(scene_id)->report() != status::unloaded) { manager::stage(scene_id, slot, exit_transition, exit_duration); finish(); } idle(elapsed_time); }
		void idle	(float elapsed_time)		override { load_icon.angle += spin_speed * elapsed_time; }
		void draw	(render_settings)	const	override { background.render(); text::out(load_text, float2{16}, float2{2}, load_font, text_color, C_BL); load_icon.render(); }
		void uninit	()							override { /*if (manager::peek(scene_id)->report() != status::active) manager::uninit(scene_id);*/ }

	};

}