#pragma once

#include <windows.h>
#include <wrl.h>
#include <d3d11.h>

#include "math.h"
#include "window.h"

#define MAX_VIEWS 8
#define VIEW_FORMAT_DEFAULT DXGI_FORMAT_R8G8B8A8_UNORM
#define VIEW_FORMAT_FLOAT DXGI_FORMAT_R16G16B16A16_FLOAT

#define FOCUS_OVERWRITE -1
#define FOCUS_DEPTH		-2
#define FOCUS_NONE		-3

#define SCREENSHOT_EXT ".png"

namespace BLIB {
	
	namespace render_target {

		// A D3DX11 Render Target View
		class view {
			// Attach all views to the shader.
			friend void bind_all();
		private:
			IDXGISwapChain* const								swap_chain_ptr;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		render_target_view;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		depth_stencil_view;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shader_resource_view;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	depth_shader_resource;
			float2												size;

			// Pipeline Cache

			view*												cached_views[MAX_VIEWS]	{ nullptr };
			view*												cached_depth			{ nullptr };
			int													active_in_slot			{ FOCUS_NONE };
			float2												cached_viewport			{};

			void create_back_buffer(D3D11_TEXTURE2D_DESC desc);
			void get_back_buffer();
			void create_depth_stencil();

		public:
			// Create a new view, by default window-sized.
			view(float2 size = window::size(), DXGI_FORMAT format = VIEW_FORMAT_DEFAULT);
			// Create the main view from the swap chain.
			view(IDXGISwapChain* swap_chain);

			// clear the render target with the background color.
			void clear(color bkg);
			void resize(float2 size);
			// Assigns the render target to capture incoming render commands in a given slot. 
			// This render target caches the previous render state.
			// If a slot is not given, defaults to FOCUS_OVERWRITE, which will capture all incoming renders.
			// To capture only depth information, use FOCUS_DEPTH.
			bool focus(int slot = FOCUS_OVERWRITE);
			// Removes the render target from the rendering pipeline.
			// Returns the render state to it's cache.
			void unfocus();
			// Release all COM objects
			void release();
			
			void copy_SRV_to		(ID3D11ShaderResourceView** dest)	const	{ shader_resource_view.CopyTo(dest); }
			void get_SRV_resource	(ID3D11Resource** out)				const	{ shader_resource_view->GetResource(out); }
			void bind_SRV			(int slot)							const	{ device::context()->PSSetShaderResources(slot, 1, shader_resource_view.GetAddressOf()); }
			void bind_depth			(int slot)							const	{ device::context()->PSSetShaderResources(slot, 1, depth_shader_resource.GetAddressOf()); }

			void save_to_file(string filename);
#ifdef _DEBUG
			ID3D11ShaderResourceView* imgui_get_SRV()		{ return shader_resource_view.Get(); }
			ID3D11ShaderResourceView* imgui_get_depth_SRV() { return depth_shader_resource.Get(); }
#endif

			float2 get_size() const { return size; }
		};

		// Initialize the main view.
		void init(IDXGISwapChain* swap_chain);
		// Clear the main view.
		void clear_main();
		// Resize the main view to a new window size.
		void resize_main(float2 size);
		// Focus the main view.
		void focus_main();
		void unfocus_main();
		// Destroy the main view.
		void release_main();
		void set_main_background(color c);

		namespace _private {
			view* get_main();
		}

		void unbind();
		void unfocus_all();

		// A helper class to focus a whole vector of views at once, and then unfocus them when finished.
		class auto_focus {
		private:
			view* views[MAX_VIEWS]{nullptr};
			void focus() { for (int i = 0; i < MAX_VIEWS; i++) { if (views[i]) views[i]->focus(i); } }

		public:
			auto_focus(const std::vector<view*>& v) {
				int count = static_cast<int>(v.size());
				assert(count <= MAX_VIEWS && count > 0);
				for (int i = 0; i < MAX_VIEWS; i++) { if (i < count) { views[i] = v[i]; } else { views[i] = nullptr; } }
				focus();
			}
			auto_focus(const std::vector<std::unique_ptr<view>>& v) {
				int count = static_cast<int>(v.size());
				assert(count <= MAX_VIEWS && count > 0);
				for (int i = 0; i < MAX_VIEWS; i++) { if (i < count) { views[i] = v[i].get(); } else { views[i] = nullptr; } }
				focus();
			}
			//auto_focus(const view* v[MAX_VIEWS])						{ for (int i = 0; i < MAX_VIEWS; i++) { views[i] = v[i];		} focus(); }
			auto_focus(const std::unique_ptr<view> v[MAX_VIEWS])		{ for (int i = 0; i < MAX_VIEWS; i++) { views[i] = v[i].get();	} focus(); }
			~auto_focus() { for (int i = MAX_VIEWS - 1; i >= 0; i--)	{ if (views[i]) views[i]->unfocus(); } }
		};

#define quick_focus(views) auto_focus _af(views)
	}

	// set the filepath to save screenshots to.
	void set_screenshot_filepath(string filepath);

	// schedule a screenshot on the main view.
	void screenshot(string filename = "screenshot");

	string empty_screenshot_queue();

}