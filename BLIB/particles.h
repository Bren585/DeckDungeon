#pragma once
#include "model.h"
#include "texture_animation.h"

/*
	The particles class is a base class on which programmers
	can build their own custom particle systems.

	Simply, it is a collection of billboards that read from
	the texture data of one sprite.
*/

#define DEFAULT_MAX_PARTICLES	128
#define MAX_MAX_PARTICLES		512

namespace BLIB {
	// Particle System
	class particles : public model, public mesh {
	private:
		// Set up shaders and buffers.
		void com_setup();

		virtual particles* clone_impl() const = 0;

		void render(const float4x4& world, const color& material_color) const override;
		// Update the particle system.
		void _update(float elapsed_time) override;
	
		bool active = true; // Will emit new particles if true, else will stop emitting particles.

		inline static const std::vector<triangle> dummy_triangles = std::vector<triangle>();

	protected:
		struct point {
			float4 position;
			float2 size		{ 1, 1 };
			float2 uv_size	{ 1, 1 };
			float2 uv_index;
		};

		struct particle {
			point p;
			float3 velocity;
			float lifetime			= 0;
			float max_lifetime;
			float2 scale			{ 1, 1 };
			texture_animator animator;
		};

		std::vector<particle>	particle_buffer;
		std::vector<point>		point_buffer;
		uint max_particles = DEFAULT_MAX_PARTICLES;

		// THESE ARE THE CORE OF THE PARTICLE SYSTEM. WHEN MAKING A CUSTOM PARTICLE SYSTEM, THESE MUST BE OVERRIDDEN.

		// This function is called once on each particle in the system every frame. 
		// Lifetime, animation, and position are automatically updated for you, so this function is used mainly for
		// changes in velocity, scale, etc.
		virtual void update_each(float elapsed_time, particle& particle) = 0;

		// This function is called once per frame while new particles may be made.
		// (I.e., while active and below max textures). 
		// This function is to control when, how many, and what kind of particles get made.
		// Add new particles directly to the particle_buffer.
		virtual void emit		(float elapsed_time) = 0;

	public:
		particles(const particles& o) : particle_buffer(o.particle_buffer), point_buffer(o.point_buffer), active(o.active), max_particles(o.max_particles) { com_setup(); }
		particles() { com_setup(); }
		virtual ~particles() {}

		void start	() { active = true; }
		void stop	() { active = false; }

		bool is_active		() { return active; }
		bool has_particles	() { return particle_buffer.size() > 0; }

		// Load a texture by filename into a texture slot. 
		// Is the slot is unset, defaults to the texture_map slot. 
		// Other slots include normal_map, ORM, and emissive.
		inline void load_texture(const string& filename, texture_type slot = texture_map)										{ materials[0].textures[slot] = std::make_unique<material_texture_file>(filename);	materials[0].textures[slot]->construct(); }
		// Make a dummy solid-color texture and save it into a texture slot. 
		// Is the slot is unset, defaults to the texture_map slot. 
		// Other slots include normal_map, ORM, and emissive.
		inline void make_texture(color c, texture_type slot = texture_map)														{ materials[0].textures[slot] = std::make_unique<material_texture_dummy>(c);		materials[0].textures[slot]->construct(); }
		// Copy the texture data stored in an SRV into a texture slot.
		// Is the slot is unset, defaults to the texture_map slot. 
		// Other slots include normal_map, ORM, and emissive.
		inline void copy_texture(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& SRV, texture_type slot = texture_map)	{ SRV.CopyTo(materials[0].textures[slot]->data->get_release_SRV());					materials[0].textures[slot]->construct(); }

		// Gets the texture in a given slot.
		// Is the slot is unset, defaults to the texture_map slot. 
		// Other slots include normal_map, ORM, and emissive.
		inline auto& get_texture(texture_type slot = texture_map) { return materials[0].textures[slot]; }

		void set_max_particles(uint count) { max_particles = count < MAX_MAX_PARTICLES ? count : MAX_MAX_PARTICLES; }

		inline float3						get_size		() const override { return float3{0}; }
		inline const std::vector<triangle>& peek_triangles	() const override { return dummy_triangles; }
		inline uint32_t						ray_collision	(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const override { return 0; }

		render_settings default_rs() const override { return { vertex_shader("billboard"), geometry_shader("billboard") }; }

	};

}