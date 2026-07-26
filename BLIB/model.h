#pragma once
#include <map>
#include <memory>
#include "mesh.h"
#include "model_animation.h"
#include "render_settings.h"

#define NO_ANIMATION "-2"

namespace BLIB {

	// Definitions

	const float4x4 coordinate_system_transforms[]{
		{-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 0:RHS Y-UP
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 1:LHS Y-UP
		{-1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 2:RHS Z-UP
		{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 3:LHS Z-UP
	};

	enum coordinate_system {
		RH_Y,
		LH_Y,
		RH_Z,
		LH_Z
	};

	// Model

	class model {
	public:
		struct constants {
			float4x4	world;
			color		material_color;
		};

	private:
		// Animation Data

		float	transition_timer	= 0;
		float	transition_duration	= 0;
		string	sequence			= NO_ANIMATION;
		string	transition			= NO_ANIMATION;

		static inline animation::keyframe temp_frame;
		
	protected:

		static inline string filepath = L""; // The default local folder that models will be loaded from.

		Microsoft::WRL::ComPtr<ID3D11Buffer>			constant_buffer;

		std::vector<mesh>								meshes;
		std::unordered_map<uint64_t, material>			materials;
		std::unordered_map<string, animation>			animations;
		coordinate_system								coord_sys;
		std::vector<D3D11_INPUT_ELEMENT_DESC>			input_element_desc;

		// Initialize shaders from the vertex shader filename.
		void create_shaders(string vs_cso);

		inline virtual model* clone_impl() const = 0;

	public:
		auto clone() const { return std::unique_ptr<model>(clone_impl()); }
		virtual ~model() = default;

		// Change the local folder that models are loaded from.
		static inline void		set_filepath(string path)	{ filepath = path; }
		// Get the local folder that models are loaded from.
		static inline string	get_filepath()				{ return filepath; }

		virtual render_settings default_rs() const { return vertex_shader("default_full"); };

		virtual void render(const float4x4& world, const color& material_color) const = 0;
		
		// Animation

		// Call both _update() and update_animation()
		void update(float elapsed_time);

		// Update prepped for inhereted classes.
		virtual void _update(float elapsed_time) {}

		// Update the state of the animation.
		void update_animation(float elapsed_time);

		// Update the meshes to match the animation.
		void update_meshes(bool force = false);

		/*
			Begin a new animation.

			string animation_name
				- the name of the animation
			float enter_time
				- the time it should take to transition to the new animation in seconds.
				 - optional. Defaults to 0 seconds.
			bool loop
				 - if the animation should loop or not.
				 - optional. Defaults to false.
			float playback
				- the playback speed of the animation.
				- optional. Defaults to 1x.
		*/
		void animate(string animation_name, float enter_time = 0.0f, bool loop = false, float playback = 1);

		// Stops all animations.
		void stop_animation() { sequence = transition = NO_ANIMATION; }

		// Returns true if animating, false if not.
		bool is_animating() const { return sequence != NO_ANIMATION; }

		// Returns the duration of the current animation, even if it's looping.
		float remaining_animation_time() const { if (is_animating()) { return animations.at(sequence).get_remaining_time(); } else { return 0; } }

		// Get the entire list of animations.
		const auto& get_animations() const { return animations; }

		// Get the name of the current animation.
		const string& get_current_animation() const { return sequence; }

		// Get the animation data of an animation by name.
		const animation& get_animation(string name) const { return animations.at(name); }

		// Get the current keyframe.
		const animation::keyframe* get_keyframe() const;

		// Textures

		// Get all of the materials used by the model.
		auto& get_textures() { return materials; }

		// Foces all materials to reload their textures.
		void force_reload_textures() { for (auto& material : materials) { material.second.force_construct(); } }

		// Physics / Collision

		inline virtual float3 get_size() const = 0;

		virtual const std::vector<triangle>& peek_triangles() const = 0;

		virtual uint32_t ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const = 0;
	
		SERIALIZE(meshes, materials, animations, coord_sys)
	};

	// Primitives

	// Set the default level of detail used when making geometric primitives.
	// Default is 12. Minimum is 3.
	void set_primitive_detail(int level = 12);

	// create a cube with custome extents min and max. (defaults to a cube with size of 1)
	model* create_cube(float3 min = float3{ -0.5f }, float3 max = float3{ 0.5f });
	model* create_sphere();
	model* create_cylinder();
	// create a capsule, given it's height and radius. Defaults to 0.5f for both values.
	model* create_capsule(float height = 0.5f, float radius = 0.5f);
	model* create_quad();
	model* create_rect_pyramid();
	// Create a billboard of a given size. Defaults to a square 1 unit wide.
	model* create_billboard(float2 size = float2{ 1 });

	// Load a texture by filename into the given texture slot of a geometric primitive. 
	// If the texture slot is not given, defaults to texture_map.
	// The aspect ratio of the loaded texture can be captured by the optional out_aspect parameter.
	void load_texture(model* dest, const string filename, texture_type slot = texture_map, float3* out_aspect = nullptr);

	class sprite;
	// Copies the texture of a sprite into the texture_map of a geometric primitive. 
	// The aspect ratio of the loaded texture can be captured by the optional out_aspect parameter.
	void copy_texture(model* dest, const sprite* spr, float3* out_aspect = nullptr);

	// FBX

	/* 
		Load an FBX model by filename. 
		const char* fbx_filename
			- the filename of the fbx model.
		bool triangulate
			- if true, the fbx model will be triangulated.
			- used for models made of quads instead of triangles.
		coordinate_system sys
			- the coordinate system the model was built in. 
	*/
	model* load_fbx(const char* fbx_filename,		bool triangulate, coordinate_system sys);

	/*
		Load an FBX model by it's corresponding cereal file.
		const char* cereal_filename
			- the cereal filename of the fbx model.
		bool full_filepath
			- if false, the engine will automatically append the global models folder to the beginning of the filepath.
			- if true, loads the model from the filename as is.
	*/
	model* load_fbx(const char* cereal_filename,	bool full_filepath = false);

	/*
		Loads animations from a given fbx file to the given fbx model.
		model* target
			- The fbx model to recieve the animations.
		const char* animation_filename
			- The filename of the fbx file that contains the animations.
		float sampling_rate
			- The desired sampling rate at which to apply the animations. 
			- Optional. Defaults to FBX's native sampling rate.
	*/
	void add_animations_fbx(model* target, const char* animation_filename, float sampling_rate = 0);

}