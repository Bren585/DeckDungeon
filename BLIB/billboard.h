#pragma once
#include "model.h"

namespace BLIB {

	// A 2D quad mesh that billboards to the camera.
	class billboard : public model, public mesh {
	protected:
		struct point {
			float4 position = float4{ 0 };
			float2 size		= float2{ 1 };
			float2 uv_size	= float2{ 1 };
			float2 uv_index = float2{ 0 };
		};

		virtual billboard* clone_impl() const override { return new billboard(*this); }
		virtual void render(const float4x4& world, const color& material_color) const override;
		inline static const std::vector<triangle> dummy_triangles = std::vector<triangle>(); // Used to avoid being ray picked. 

	public:
		billboard(float2 size = float2{ 1 });

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

		inline float3						get_size		() const override { return float3{0}; }
		inline const std::vector<triangle>& peek_triangles	() const override { return dummy_triangles; }
		inline uint32_t						ray_collision	(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const override { return 0; }

		render_settings default_rs() const override { return { vertex_shader("billboard"), geometry_shader("billboard") }; }
	};

}