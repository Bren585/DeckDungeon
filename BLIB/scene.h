#pragma once

#include "status.h"
#include "canvas.h"
#include "window.h"
#include "manager.h"
#include "lights.h"

#define DEFAULT_BACKGROUND		BLACK

/*

	scene derive from status and canvas.
	In other words, they contain both logic and graphics.

	Unlike a status, a scene is asleep by default. When a scene
	is staged, it is automatically woken by the manager.

	When making a custom scene, the following functions must be overridden:
	
	status::init
		- called async while being initialized. As much as possible, 
		  initialization should be done here instead of the constructor.
	status::update
		- called every frame while awake. The "logic" of the scene.
	status::idle
		- called every frame while asleep.
	try_stop()
		- called every frame while attempting to force stop.
	scene::draw
		- all render calls should be made inside this function.

	The following functions are available to be overridden:

	status::uninit
		- called just before the scene is destroyed. This is an
		  opportunity to release resources or stop processes.
	status::on_wake
		- called when the scene is woken.
	status::on_sleep
		- called when the scene is put to sleep.

*/

namespace BLIB {

	class camera;

	namespace generic { class scene; }
	namespace lighting { void make_shadows(const generic::scene*, const std::vector<light>*); }

	namespace generic {
		class scene : public status, public canvas {
			friend void lighting::make_shadows(const generic::scene*, const std::vector<light>*);
		private:
			camera*				active_camera	= nullptr;
			std::vector<light>	lights;
			environment_lights	scene_lights;

			virtual void _on_resize() {} // engine use only
			void on_load() override { force_sleep(); }

		protected:
			// All render calls should be made inside this function.
			virtual void draw(render_settings = {}) const	{}
			// This is called when the window is resized. UI changes should be made here.
			virtual void on_resize()						{}

		public:
			scene(float2 size = window::size()) : canvas(size) {
				pivot = C_BL;
				set_background(DEFAULT_BACKGROUND);
			}

			void resize(float2 size) { canvas::resize(size); _on_resize(); on_resize(); }

			void						set_post_effects	(render_settings rs)	{ object::set_settings(rs); }
			void						add_post_effects	(render_settings rs)	{ object::add_settings(rs); }
			render_settings				get_post_effects	() const				{ return object::get_settings(); }

			void						set_camera			(camera* cam)			{ active_camera = cam;	}
			camera*						get_camera			()						{ return active_camera; }
			const camera*				get_camera			() const				{ return active_camera; }

			environment_lights&			get_scene_lights	()						{ return scene_lights; }
			const environment_lights&	get_scene_lights	() const				{ return scene_lights; }

			std::vector<light>&			get_lights			()						{ return lights; }
			const std::vector<light>&	get_lights			() const				{ return lights; }

			virtual ~scene() = default;

			// Called asynchronously the first time the scene is ticked. 
			virtual void init	()						override = 0;
			// Called every frame while awake.
			virtual void update	(float elapsed_time)	override = 0;
			// Called every frame while asleep
			virtual void idle	(float elapsed_time)	override = 0;

			////	inherited methods from status
			// virtual void try_stop() {}
			// 
			// virtual void uninit() {}
			// virtual void on_wake() {}
			// virtual void on_sleep() {}
			// virtual void on_stop() {}

			virtual void _render(const camera* cam) const = 0;
			virtual void render(const camera* cam = nullptr, const environment_lights* scene_lights = nullptr, const std::vector<light>* lights = nullptr) const;
		};
	}

	namespace flat {
		// A scene containing only 2D elements
		class scene : public generic::scene {
			friend class camera_scene;
		private:
			void _render(const camera* cam) const override;
		public:
			scene(float2 size = window::size()) : generic::scene(size) {}
			virtual ~scene() {}
		};
	}

	namespace full {
		// A scene containing 3D elements.
		class scene : public generic::scene {
		private:
			static constexpr UINT geometry_layer_count = 5;

			Microsoft::WRL::ComPtr<ID3D11Buffer> point_buffer;
			mutable Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;
			std::unique_ptr<render_target::view> geometry_buffer[MAX_VIEWS];
			void _on_resize() override { for (int i = 0; i < geometry_layer_count; i++) geometry_buffer[i]->resize(get_view_size()); }

			void _render(const camera* cam) const override;

		protected:
			// Render geometry that casts shadows.
			void opaque_pass		(const camera* cam) const;
			// Renders lighting and shadows.
			void lighting_pass		()					const;
			// Renders geometry that does not interact with light.
			void transparent_pass	(const camera* cam) const;

			// Like draw, this is called every frame. Render calls for objects that do not cast shadows should be made here to improve performance.
			virtual void draw_transparent() const {}

		public:
			scene(float2 size = window::size()) : generic::scene(size) {
				geometry_buffer[0] = std::make_unique<render_target::view>(size);						// Albedo
				geometry_buffer[1] = std::make_unique<render_target::view>(size);						// Normal
				geometry_buffer[2] = std::make_unique<render_target::view>(size);						// ORM
				geometry_buffer[3] = std::make_unique<render_target::view>(size);						// Emissive
				geometry_buffer[4] = std::make_unique<render_target::view>(size, VIEW_FORMAT_FLOAT);	// World Position
				geometry_buffer[5] = nullptr;
				geometry_buffer[6] = nullptr;
				geometry_buffer[7] = nullptr;
				make_point_buffer(point_buffer.GetAddressOf());
				make_constant_buffer(constant_buffer.GetAddressOf());
				update_point_buffer(point_buffer.Get(), float2{ 0 }, float2{ 1 }, C_BL, 0, float2{ 0 }); // Fill the point buffer with one single point.
			}
			virtual ~scene() {}

#ifdef _DEBUG
			render_target::view* get_gbuffer_slice(int i) { return geometry_buffer[i].get(); }
#endif
		};
	}

	// A scene that only exists to take a picture of another 3D scene.
	class camera_scene : protected flat::scene {
	private:
		generic::scene* target_scene;

	public:
		camera_scene(float2 size = window::size(), generic::scene* scene = nullptr) : flat::scene(size), target_scene(scene) {}

		void set_scene(generic::scene* scene) { target_scene = scene; }

		void draw	(render_settings)				const override;
		void render	(const camera* cam = nullptr,	const environment_lights* scene_lights = nullptr, const std::vector<light>*lights = nullptr)	const override;

	};
}