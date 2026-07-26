#pragma once
#include "sprite_batch.h"
#include "math.h"
#include "string.h"
#include "cereal.h"

/*
	Fonts are generated via https://evanw.github.io/font-texture-generator/
	Then, the black backgrounds are converted to transparent, leaving a pure white, semitransparent jpg.

	The font metadata should then be saved as-is into a json file, with the same name as the jpg.
*/

#define TEXT_CHAR_LIMIT		256
#define FONT_DEFAULT		"Arial"

namespace BLIB {

	namespace text {

		// Render text to the active render_target::view
		float	out(string s, float2 pos, float2 scale, string font_name = FONT_DEFAULT, color color = { 1.0f, 1.0f, 1.0f, 1.0f }, float2 align = {-1, -1});

		// Set the default local folder from which fonts are loaded.
		void	set_filepath(string path);

		// Get the width of a string if it were typed in a given font.
		float	width(string s, string font_name = FONT_DEFAULT);

		// Get the height of a given font.
		float	height(string font_name = FONT_DEFAULT);

		class font : public sprite_batch {
		private:
			struct metadata {
				struct glyph {
					float2 pos;		// Where the glyph is stored
					float2 size;	// The size of the glyph
					float2 offset;	// Where the glyph is positioned relative to the cursor
					float advance;	// How far the cursor should move after this glyph
					SERIALIZE(pos, size, offset, advance)
				};

				float font_size = 0;
				std::unordered_map<char, glyph> atlas{};
				SERIALIZE(font_size, atlas)
			};

			metadata meta{};
			string buffer;

			// Inherited from sprite_batch. Convert the string buffer to individual sprite::vertices.
			void render_all(float2 pos, float2 scale, float2 pivot, float rotation) override;
			// Load the font metadata from the .json file.
			void load_metadata(string name);
		public:
			font(string name);

			// Set the text to be typed.
			void set_buffer(string b) { buffer = b; }

			float get_font_size() { return meta.font_size; }
			// Get the width of a string if it were typed in this font.
			float get_width(string s);
			// Get the width of the current text to be typed.
			float get_buffer_width() { return get_width(buffer); }
		};

		// Empty the cached fonts.
		void uninit();

	}
}