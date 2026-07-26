#pragma once
#include "interfaces.h"
#include "sprite.h"
#include "model.h"
#include "collider.h"
#include "texture_animation.h"
#include <memory>
#include "cereal.h"

/*
	An object is a static container for a model or sprite and 
	it's location data. It also contains a collider, as well as
	render_settings for the asset.

	Objects have generic (base), flat (2D) and full (3D) 
	implementations. 

	Object's location information (position, rotation, scale) 
	is public.

	The size of an object is determined by the size of the 
	underlying asset and it's scale.
*/

namespace BLIB {

	namespace generic {
		class object : public renderable {
		private:
			render_settings default_settings;		// The default render settings to be used on this object.
		protected:
			std::unique_ptr<collider> collision;
			// Sync the collider to this object.
			virtual void sync_collider() = 0;

			// Update the collider and animations.
			virtual void _basic_update(float elapsed_time) = 0;
			// Render the asset.
			virtual void _render(render_settings settings) const = 0;

		public: // Inherit as Protected
			color tint;

		public: // Redefine as Public
			object(color c = WHITE) : tint(c) {}
			object(const object& o) : tint(o.tint), collision(o.peek_collider() ? o.peek_collider()->clone() : nullptr) {}
			virtual ~object() = default;
			inline void render(render_settings settings = {}) const { _render(default_settings & settings); }

			inline void update(float elapsed_time) { _basic_update(elapsed_time); sync_collider(); }

			// Set new default render settings.
			void set_settings(render_settings settings) { default_settings  = settings; }
			// Add new default settings to the existing render settings.
			void add_settings(render_settings settings) { default_settings &= settings; }
			// Get the current default render settings.
			render_settings get_settings() const { return default_settings; }

			// Assign a collider to the object. 
			// This function TAKES OWNERSHIP of the collider.
			virtual void			set_collider	(collider* c) = 0;
			virtual const collider* peek_collider	() const = 0;
			virtual collider*		get_collider	() = 0;

			SERIALIZE(default_settings, tint)
		};
	}

	namespace flat {

		// A 2D Object with a Sprite.
		class object : public generic::object {
		private:
			std::unique_ptr<sprite> spr;
			string filename;
			float2 size;
			texture_animator animator;

		protected:
			void sync_collider() override {
				if (!collision) return;
				static_cast<collider*>(collision.get())->sync(pos, scale, angle);
			}

			virtual void _render(render_settings settings) const override { if (spr) { settings.set(); spr->render(pos, scale, pivot, angle, tint, animator.get_frame(), size); } }
			virtual void _basic_update(float elapsed_time) override { animator.update(elapsed_time); }

			// ONLY FOR USE WITH CANVAS::RESIZE 
			void resize(float2 s) { size = s; } 

		public: // Inherit as Protected
			float2	pos;
			float	angle;
			float2	scale;
			float2	pivot; // Defines the "center point" of the object, or, the pivot point around which it rotates. Default is {0, 0} (C_BL).

			sprite* peek_sprite() { return spr.get(); }
			texture_animator& peek_animator() { return animator; }

			object()											: spr(nullptr),														angle(0),		scale(1),		pivot(C_BL)														{ set_settings(render_settings{ pixel_shader("default_flat")						}); size = float2{1};								}
			// Create an object with a sprite from a file. Optionally, input the size of a tiled sprite in the tile_size parameter.
			object(string file, float2 tile_size = float2{0})	: spr(std::make_unique<sprite>(file)),		filename(file),			angle(0),		scale(1),		pivot(C_BL)														{ set_settings(render_settings{ pixel_shader("default_flat") } &sprite::default_rs(	)); size = tile_size ? tile_size : spr->get_size(); }
			object(const object& o)								: spr(o.spr ? o.spr->clone() : nullptr),	filename(o.filename),	angle(o.angle), scale(o.scale),	pivot(o.pivot), pos(o.pos), size(o.size), generic::object(o)	{ set_settings(o.get_settings()); }
			virtual ~object() = default;

			object operator=(const object& o) { if (o.spr) { spr.reset(o.spr->clone()); } else { spr.reset(); } filename = o.filename; pos = o.pos; size = o.size; angle = o.angle; pivot = o.pivot; tint = o.tint; return o; }

		public: // Redefine as Public after Protected Inheritence
			
			// Loads a new sprite via a filename.
			void load_sprite(string file,			float2 tile_size = float2{0}	)	{ filename = file;		spr = std::make_unique<sprite>(file);				size = (bool)tile_size ? tile_size :	spr->get_size();	add_settings(sprite::default_rs());		}
			// Assign a sprite to the object. This function TAKES OWNERSHIP of the sprite.
			void set_sprite	(sprite* new_sprite,	float2 tile_size = float2{0}	)	{						spr.reset(new_sprite);		if (spr)			{	size = (bool)tile_size ? tile_size :	spr->get_size();	add_settings(sprite::default_rs()); }	}
			// Copies the given sprite and assigns the copy to the object. This function DOES NOT take ownership of the sprite.
			void copy_sprite(sprite* old_sprite,	float2 tile_size = float2{0}	)	{ assert(old_sprite);	spr.reset(old_sprite->clone());						size = (bool)tile_size ? tile_size :	spr->get_size();	add_settings(sprite::default_rs());		}
			// Make a dummy sprite for the object. The sprite will be a solid color rectangle.
			void make_dummy	(color c												)	{						spr = std::make_unique<sprite>(c, float2{1.0f});	size =									{1.0f, 1.0f};		add_settings(sprite::default_rs());		}

			// Get the size of the object's sprite (or, the tile size).
			inline float2	get_true_size	() const									{ if (spr) return size; else return {0, 0};	}
			// Change the tile size of a tiled sprite.
			inline void		set_tile_size	(float2 s)									{ scale *= size / s; size = s;				}
			// Get the size of the object, including scale.
			inline float2	get_size		() const									{ return get_true_size() * scale;			}
			// Adjust the scale of the object to match a given size.
			inline void		set_size		(float2 s)									{ if (spr) scale = (s / size);				}

			// Assign a collider to the object. 
			// This function TAKES OWNERSHIP of the collider.
			void			set_collider	(generic::collider* c)	override			{ collider* temp = dynamic_cast<collider*>(c); assert(temp); collision.reset(temp);		}
			const collider* peek_collider	() const				override			{ return static_cast<collider*>(collision.get());										}
			collider*		get_collider	()						override			{ return static_cast<collider*>(collision.get());										}
		};
	}

	namespace full {
		// A 3D Object with a Model.
		class object : public generic::object {
		private:
			std::unique_ptr<model>	mdl;
			transform				trans;

		protected:
			void sync_collider() override {
				if (!collision) return; 
				static_cast<collider*>(collision.get())->sync(trans);
			}
			
			virtual void _render(render_settings settings) const override { if (mdl) { settings.set(); mdl->render(trans, tint); } }
			virtual void _basic_update(float elapsed_time) override { if (mdl) mdl->update(elapsed_time); }

		public: // Inherit as Protected
			object()				: mdl(nullptr)												{ set_settings({ pixel_shader("default_full") });	}
			object(const object& o) : mdl(o.mdl->clone()), trans(o.trans), generic::object(o)	{ set_settings(o.get_settings());					}
			virtual ~object() = default;

			object& operator=(const object& o) { mdl = (o.mdl ? o.mdl->clone() : nullptr); trans = o.trans; tint = o.tint; return *this; }

		public: // Redefine as Public after Protected Inheritence

			model*			get_model()						{ return mdl.get(); }
			const model*	get_model() const				{ return mdl.get(); }

			// Assign a model to the object. This function TAKES OWNERSHIP of the model.
			void			set_model(model* m)				{ mdl.reset(m);		if (mdl) add_settings(mdl->default_rs()); }
			// Copies the given model and assigns the copy to the object. This function DOES NOT take ownership of the model.
			void			copy_model(const model* m)		{ mdl = m->clone(); if (mdl) add_settings(mdl->default_rs()); }

			transform		get_trans()	const				{ return trans; }
			void			set_trans(const transform& t)	{ trans = t; }

			// Get the size of the object, including scale.
			virtual float3	get_size() const { if (mdl) return mdl->get_size() * trans.get_scl(); else return float3(0); }

			// Assign a collider to the object. 
			// This function TAKES OWNERSHIP of the collider.
			void			set_collider	(generic::collider* c)	override { collider* temp = dynamic_cast<collider*>(c); assert(temp); collision.reset(temp); }
			const collider* peek_collider	() const				override { return static_cast<collider*>(collision.get()); }
			collider*		get_collider	()						override { return static_cast<collider*>(collision.get()); }

			float3		get_mid() const { return trans.get_mid(); }
			float3		get_pos() const { return trans.get_pos(); }
			quaternion	get_qtn() const { return trans.get_qtn(); }
			float3		get_scl() const { return trans.get_scl(); }
			float3		get_pvt() const { return trans.get_pvt() / get_size() * 2.0f; } // convert clip space to world space

#ifdef _DEBUG
			float3&		imgui_get_pos() { return trans.get_pos_ref(); }
			quaternion& imgui_get_qtn() { return trans.get_qtn_ref(); }
			float3&		imgui_get_scl() { return trans.get_scl_ref(); }
			float3&		imgui_get_pvt() { return trans.get_pvt_ref(); }

			void imgui_force_update() { trans.force_update(); }
#endif

			void set_pos(float3		p) { trans.set_pos(p); }
			void set_ang(float3		a) { trans.set_ang(a); }
			void set_qtn(quaternion q) { trans.set_qtn(q); }
			void set_scl(float3		s) { trans.set_scl(s); }
			void set_pvt(float3		p) { trans.set_pvt(p * get_size() * 0.5f); } // convert clip space to world space

			void add_pos(float3		d) { trans.add_pos(d); }
			void add_ang(float3		d) { trans.add_ang(d); }
			void add_scl(float3		d) { trans.add_scl(d); }
			void add_pvt(float3		p) { trans.add_pvt(p * get_size() * 0.5f); } // convert clip space to world space

			void mlt_pos(float3		d) { trans.mlt_pos(d); }
			void mlt_qtn(quaternion	d) { trans.mlt_qtn(d); }
			void mlt_scl(float3		d) { trans.mlt_scl(d); }
			void mlt_pvt(float3		p) { trans.mlt_pvt(p * get_size() * 0.5f); } // convert clip space to world space

			//bool ray_collision(const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const { return mdl->ray_collision(trans, origin, ray, out_int_point, out_int_normal, any_hit); }
		};

	}
}