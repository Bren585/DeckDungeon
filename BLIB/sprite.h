#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "string.h"
#include "math.h"
#include "render_settings.h"

// These are the default sprite shaders

#define SPRITE_VS DEFAULT_FLAT
#define SPRITE_GS DEFAULT_FLAT

// These are used to change the color and size of individual sprites, i.e. in a sprite batch.

#define VARIABLE_VS "variable_flat"
#define VARIABLE_GS "variable_flat"

namespace BLIB {

	class sprite {
	public:
		struct vertex {
			float2	position;
			float2	scale;
			float2	pivot;
			float	rotation;
			float2	tile_index; // or uv_position
			float2	tile_size;	// or uv_size, used by variable_flat only
			color   tint;		// variable_flat only
		};

		struct constants {
			color	color;
			float2  tile_size;	// uv_size, unused by variable_flat
			float2  texture_size;
			float2  viewport;
			uint    y_invert;
			float   dummy;
		};

		enum flags {
			no_flags		= 0,
			full_filename	= 1 << 0, // If false, the default sprite filepath will be appended to the filename
			make_vbuffer	= 1 << 1, // Create a vertex buffer
			make_cbuffer	= 1 << 2, // Create a constants buffer
			load_texture	= 1 << 3, // Load a texture from a file
			load_shaders	= 1 << 4, // Load the shaders
			variable		= 1 << 5, // Has dynamic size and color

			make_buffers	= make_vbuffer | make_cbuffer, // Make both vertex and constant buffers

			clone_flags		= make_buffers,											// To use when making a clone of a sprite
			dummy_flags		= load_shaders	| make_buffers,							// To use when making a dummy sprite
			canvas_flags	= load_shaders	| make_buffers,							// To use when making a canvas
			batch_flags		= load_shaders	| load_texture		| make_cbuffer,		// To use when making a sprite_batch
			font_flags		= batch_flags	| full_filename		| variable,			// To use when making a font
			default_flags	= load_shaders	| make_buffers		| load_texture		// To use when making a default sprite
		};

	private:
		static bool		y_invert; // 2D Computer graphics convention states that +Y is down. I dislike this. y_invert effectively changes +Y to up instead. Toggleable via sprite::set_y_invert
		static float2	viewport; // The viewport for rendering purposes. It's automatically handled by render_target::view, so don't touch it unless you really know what you're doing.
		static string	filepath; // The default local folder to load sprites from.

		void create_vertex_buffer();
		void create_constant_buffer();

	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				constant_buffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shader_resource_view	= nullptr;
		D3D11_TEXTURE2D_DESC								texture2d_desc{};

		void update_vertex_buffer(float2 pos, float2 scale, float2 pivot, float rotation, float2 tile_index);
		void update_constant_buffer(color color, float2 tile_size);

	public:
		sprite(flags flags, const string& filename = "");
		sprite(const string& filename) : sprite(default_flags, filename) {}
		sprite(color c, float2 size);
		sprite(const sprite&) = delete;
		sprite(sprite&&) = default;
		~sprite() {}

		virtual void	load_shader(string vs);
		void			load_file(const string& filename, bool full_filepath);

		// Make a new clone of this sprite. 
		sprite* clone() const;

		ID3D11ShaderResourceView**								get_release_SRV	()			{ return shader_resource_view.Get() ? shader_resource_view.ReleaseAndGetAddressOf() : shader_resource_view.GetAddressOf(); }
		ID3D11ShaderResourceView**								get_SRV			()			{ return shader_resource_view.Get() ? shader_resource_view.GetAddressOf()			: nullptr; }
		const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& peek_SRV		() const	{ return shader_resource_view; }

		static render_settings default_rs() { return { vertex_shader{ SPRITE_VS }, geometry_shader{ SPRITE_GS } }; }

		//static vertex_shader		default_vs() { return vertex_shader		{ SPRITE_VS	}; }
		//static geometry_shader	default_gs() { return geometry_shader	{ SPRITE_GS }; }

		// If true, +Y is up. Else, +Y is down.
		static void		set_y_invert(bool invert)	{ y_invert = invert; }
		// Don't touch this if you aren't a render_target::view.
		static void		set_viewport(float2 size)	{ viewport = size; }
		// Change the defualt local folder where sprites are loaded from.
		static void		set_filepath(string path)	{ filepath = path; }
		
		static bool		get_y_invert()				{ return y_invert; }
		static float2	get_viewport()				{ return viewport; }
		static string	get_filepath()				{ return filepath; }

		float2			get_size	() const		{ return { (float)texture2d_desc.Width, (float)texture2d_desc.Height }; }
		// Change what size the sprite thinks it is. This does not actually change the size of the sprite.
		void			resize		(float2 size)	{ texture2d_desc.Width = (uint)size.x; texture2d_desc.Height = (uint)size.y; }

		virtual void render(float2 pos, float2 scale, float2 pivot, float rotation, color color, float2 tile_index, float2 tile_size);
	};

	// Create a buffer thats a collection of single sprite::vertex objects, instead of a polygon.
	void make_point_buffer(ID3D11Buffer** out);

	void update_point_buffer(ID3D11Buffer* point_buffer, float2 pos = sprite::get_viewport() / 2.0f, float2 scale = float2{1}, float2 pivot = C_CC, float rotation = 0, float2 tile_index = float2{0});

	void make_constant_buffer(ID3D11Buffer** out);

	void update_constant_buffer(ID3D11Buffer** constant_buffer_addr, float2 texture_size, color c = WHITE, float2 tile_size = sprite::get_viewport());

	// Render the contents of a point buffer to the screen. See: make_point_buffer
	void draw_points(ID3D11Buffer* const* vertex_buffer_adr, uint count = 1);

}