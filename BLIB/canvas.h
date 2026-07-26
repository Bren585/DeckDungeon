#pragma once
#include "object.h"
#include "render_target.h"
#include "text.h"

namespace BLIB {
	// A 2D object that contains a render target.
	class canvas : public flat::object {
	private:
		mutable render_target::view view;
		color background;

	public:
		canvas(float2 size = {1, 1});

		// clear the render target with the background color.
		void clear() const;

		// resize the object and the underlying render target.
		void resize(float2 size);

		// get the size of the underlying render target.
		float2 get_view_size() const { return view.get_size(); }

		//void _render(render_settings settings) const override { assert((bool)get_size()); flat::object::_render(settings); }

		// draw a renderable to the render target with optional render settings.
		void draw(const renderable* r, render_settings rs = {}) const;

		/*
			Use BLIB Engine's text renderer to type text to the render target.
			Returns the height of the rendered text for ease of multi-line typing.

			string s
				- The text to be typed.
			float2 pos
				- The position of the text.
				- By default, this is the bottom left corner of the text. This can be changed with the align parameter.
			float2 scale
				- The scale of the font.
			string font
				- The font name. 
				- Optional. Defaults to the font defined by FONT_DEFAULT.
			color color
				- The color of the text.
				- Optional. Defaults to white.
			float2 align
				- The alignment of the text origin. 
				- Optional. Defaults to bottom left (C_BL).
				- Ranges from {-1, -1} (or C_BL) to {1, 1} (or C_TR).
		*/
		float type(string s, float2 pos, float2 scale, string font = FONT_DEFAULT, color color = { 1.0f, 1.0f, 1.0f, 1.0f }, float2 align = { -1, -1 }) const;
		
		// Assigns the render target to capture incoming render commands in a given slot. 
		// This render target caches the previous render state.
		// If a slot is not given, defaults to FOCUS_OVERWRITE, which will capture all incoming renders.
		// To capture only depth information, use FOCUS_DEPTH.
		bool focus	(int slot = FOCUS_OVERWRITE)	const { return view.focus(slot);	}

		// Removes the render target from the rendering pipeline.
		// Returns the render state to it's cache.
		void unfocus()								const { view.unfocus();				}

		operator render_target::view*				()			{ return &view; }
		render_target::view*			get_view	()			{ return &view; }
		const render_target::view&		peek_view	() const	{ return view;	}

		// Render the contents of the render target to the main screen.
		void render_to_main() const {
			render_target::focus_main();
			flat::object::render();
		}

		void	set_background(color c)			{ background = c; }
		color	get_background()		const	{ return background; }	
		
		// Copy the image on the canvas to a sprite.
		void snapshot_to_sprite	(sprite*	target		) const;

		// Save the image on the canvas to a file.
		void snapshot_to_file	(string		filename	) { view.save_to_file(filename); }
		
	};
}