#pragma once
#include <d3d11.h>
#include "string.h"
#include "math.h"

namespace BLIB {
	namespace texture {
		// Load a texture from a file into an SRV
		HRESULT load_file(const wchar_t* filename, ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc);
		// Load a texture from a file into an SRV
		inline HRESULT load_file(const string& filename, ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc) { return load_file((filename).wide(), shader_resource_view, texture2d_desc); }

		// Make a dummy SRV of the given color and size
		HRESULT make_dummy(ID3D11ShaderResourceView** shader_resource_view, color value, float2 dimension);

		// Save the contents of a Resource to a file
		HRESULT save_to_file(ID3D11Resource* resource, string filename);

		// Save the contents of an SRV to a file
		HRESULT save_to_file(ID3D11ShaderResourceView* shader_resource_view, string filename);

		// Release cached textures
		void release_all();
	}
}

