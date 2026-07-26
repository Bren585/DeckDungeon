#pragma once
#include "sprite.h"

/*
	A Sprite batch is a more efficient way to print a large number of sprites out of the same spritesheet.

	Think confetti, or a tilemap. 

	To use a sprite_batch, you need to create a new class that inherits sprite_batch, then override render_all.
	render_all should contain the logic for where and what kind of sprites to render. For each sprite of the 
	batch, you must call the prerender function.

	For an example of this in practice, look at the font class in text.h
*/

namespace BLIB {

	class sprite_batch : public sprite {
	private:
		const size_t max_vertices;
		std::vector<vertex> vertices;

		void begin(color color, float2 tile_size);
		void end();

	protected:

		// To be overritted by implementations of sprite_batch. Loads the data of all of the individual sprites to be rendered.
		virtual void render_all(float2 pos, float2 scale, float2 pivot, float rotation) = 0;
		
		/*
			Prepare data to be rendered later.

			float2 pos
				- The X and Y position of the sprite on the screen.
			float2 scale
				- The X and Y scale of the sprite, relative to it's tile_size.
			float2 pivot
				- A signifier of what the position of the sprite "means," or, the point at which the sprite rotates.
				- Values range from {-1, -1} (or C_BL) to {1, 1} (or C_TR)
			float rotation
				- The rotation of the sprite around the pivot.
			float2 tile_index
				- Using the tile_size as a base, the X and Y indicies of the tile.
				- Or, where to start reading from the texture file.
			float2 tile_size
				- Used by variable sprites only, to dynamically change the tile_size.
				- Or, how much data to read from the texture file.
				- Optional. Defaults to {0, 0}.
		*/
		void prerender(float2 pos, float2 scale, float2 pivot, float rotation, float2 tile_index, float2 tile_size = float2{0});

	public:
		sprite_batch(const string& filename, size_t max_sprites, flags flags = batch_flags);
		sprite_batch(sprite_batch&& o) noexcept;
		sprite_batch& operator= (sprite_batch&&) = default;
		~sprite_batch() {}

		void render(float2 pos, float2 scale, float2 pivot, float rotation, color color, float2 tile_index, float2 tile_size) override {
			begin(color, tile_size);
			render_all(pos, scale, pivot, rotation);
			end();
		}
	};
}