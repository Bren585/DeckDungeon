#pragma once
#include "cereal.h"
#include "sprite.h"
#include "device.h"
#include <array>

/* 
	Materials contain the texture data for a model, including
	their alpha, occlusion, roughness, metallic, and emissive.
*/

#define DEFAULT_TEXTURE_MAP		WHITE
#define DEFAULT_NORMAL_MAP		color {0.5f, 0.5f, 1.0f}
#define DEFAULT_OCCLUSION		1.0f
#define DEFAULT_ROUGHNESS		0.5f
#define DEFAULT_METALLIC		0.0f
#define DEFAULT_ORM_MAP			color {DEFAULT_OCCLUSION, DEFAULT_ROUGHNESS, DEFAULT_METALLIC}
#define DEFAULT_EMISSION_MAP	COLORLESS

namespace BLIB {

	enum texture_type {
		texture_map,
		normal_map,
		ORM,
		emissive,

		texture_count,
	};

	// An intermediary struct used to convert phong materials into BLIB materials. 
	struct phong {
		uint64_t	unique_id{ 0 };
		string		name;

		color Ka{ 0.2f, 0.2f, 0.2f, 1.0f };
		color Kd{ 0.8f, 0.8f, 0.8f, 1.0f };
		color Ks{ 1.0f, 1.0f, 1.0f, 1.0f };

		string texture_filenames[4];

		SERIALIZE(unique_id, name, Ka, Kd, Ks, texture_filenames)
	};

	// A base material component. 
	// Texture data is saved in a sprite.
	struct material_texture {
		std::unique_ptr<sprite> data{ nullptr };
		material_texture() = default;
		material_texture(const material_texture&) = delete;
		material_texture(material_texture&&) = default;
		// initialize texture data if it does not exist.
		void construct() { if (!data) force_construct(); }
		// force re-initialization of texture data, even if it already exists.
		virtual void force_construct() {}
		virtual ~material_texture() = default;
		SERIALIZE_EMPTY()
	};

	// A material component that is constructed from a file.
	struct material_texture_file : material_texture {
		string filename;
		material_texture_file() = default;
		// construct a material component from a single file.
		material_texture_file(string filename) : filename(filename) {}
		material_texture_file(const material_texture_file&) = delete;
		material_texture_file(material_texture_file&&) = default;
		// force re-initialization of texture data, even if it already exists.
		void force_construct() override { if (!data) data = std::make_unique<sprite>(sprite::load_texture, filename); }
		SERIALIZE_BASE(material_texture, filename)
	};

	// Load a bump map from a file and convert it to a normal map, then save the new texture into a sprite.
	void construct_normal_from_bump(std::unique_ptr<sprite>& normal_map, string filename);

	// A material component that is constructed from a bump (height) map.
	struct material_texture_height : material_texture_file {
		material_texture_height() = default;
		// construct a material component from a bump (height) map.
		material_texture_height(string filename) : material_texture_file(filename) {}
		material_texture_height(const material_texture_height&) = delete;
		material_texture_height(material_texture_height&&) = default;
		// force re-initialization of texture data, even if it already exists.
		void force_construct() override { construct_normal_from_bump(data, filename); }
		SERIALIZE_EMPTY_BASE(material_texture_file)
	};

	// A dummy material component that is constructed from a solid color.
	struct material_texture_dummy : material_texture {
		color c;
		material_texture_dummy() = default;
		// construct a material component from a solid color.
		material_texture_dummy(color c) : c(c) {}
		material_texture_dummy(const material_texture_dummy&) = delete;
		material_texture_dummy(material_texture_dummy&&) = default;
		// force re-initialization of texture data, even if it already exists.
		void force_construct() override { data = std::make_unique<sprite>(c, float2{ 1.0f }); }
		SERIALIZE_BASE(material_texture, c)
	};

	// A material component that is constructed from three files.
	// The first file is the Occlusion map.
	// The second file is the roughness map.
	// The third file is the metallic map.
	// If a file is not provided, the corresponding value 
	// of the fallback color will be used to create a dummy.
	// For recommended fallback values, see DEFAULT_ORM_MAP
	struct material_texture_unpacked_orm : material_texture {
		color c{ 0, 0, 0 };
		string filenames[3]{};
		material_texture_unpacked_orm() = default;
		// Construct a material component from three files.
		// The first file is the Occlusion map.
		// The second file is the roughness map.
		// The third file is the metallic map.
		// If a file is not provided, the corresponding value 
		// of the fallback color will be used to create a dummy.
		// For a recommended fallback color, use DEFAULT_ORM_MAP.
		material_texture_unpacked_orm(color c, string files[3]) : c(c) { for (int i = 0; i < 3; i++) { filenames[i] = files[i]; } }
		material_texture_unpacked_orm(const material_texture_unpacked_orm&) = delete;
		material_texture_unpacked_orm(material_texture_unpacked_orm&&) = default;
		// force re-initialization of texture data, even if it already exists.
		void force_construct() override;
		SERIALIZE_BASE(material_texture, c, filenames[0], filenames[1], filenames[2])
	};

	inline std::unique_ptr<material_texture_dummy> create_default_material(texture_type type) {
		switch (type) {
		case normal_map:			return std::make_unique<material_texture_dummy>(DEFAULT_NORMAL_MAP		);
		case ORM:					return std::make_unique<material_texture_dummy>(DEFAULT_ORM_MAP			);
		case emissive:				return std::make_unique<material_texture_dummy>(DEFAULT_EMISSION_MAP	);
		default /*texture_map*/:	return std::make_unique<material_texture_dummy>(DEFAULT_TEXTURE_MAP		);
		}
	}

	struct material {

		uint64_t unique_id{ 0 };
		string name;
		std::array<std::unique_ptr<material_texture>, texture_count> textures = { nullptr };

		material() = default;
		material(material&&) = default;
		material(const material& o) {
			unique_id = o.unique_id;
			name = o.name;
			for (int i = 0; i < texture_count; i++) {
				textures[i] = std::make_unique<material_texture>();
				textures[i]->data.reset(o.textures[i]->data->clone());
			}
		}

		// initialize all material textures, or create default textures.
		void construct() {
			for (int t = 0; t < texture_count; t++) {
				auto& tex = textures[t];
				if (!tex) { tex = create_default_material(texture_type(t)); }
				tex->construct();
			}
		}

		// force re-initialization of material textures, or create default textures.
		void force_construct() {
			for (int t = 0; t < texture_count; t++) {
				auto& tex = textures[t];
				if (!tex) { tex = create_default_material(texture_type(t)); }
				tex->force_construct();
			}
		}

		// attach the material to the shader.
		void bind(int slot) const {
			for (int i = 0; i < 4; i++) {
				device::context()->PSSetShaderResources(slot + i, 1, textures[i]->data->get_SRV());
			}
		}

		SERIALIZE(unique_id, name, textures)
	};

	// convert phong data into a material.
	void construct_pbr_from_phong(material* mat, color Ka, color Kd, color Ks);

}