#pragma once
#include "math.h"
#include "window.h"
#include <d3d11.h>

/*
	A camera is neccesary to render images to the screen. 

	There are two types of cameras built into the BLIB Engine, 
	Perspective amd Orthographic cameras.

	For most 3D Applications, a Perspective Camera is recommended.
*/

// Perspective Defaults
#define P_EYE_DEFAULT		{ 0.0f, 0.0f, -10.0f }
#define P_FOCUS_DEFAULT		{ 0.0f, 0.0f, 0.0f }
#define P_CLIP_DEFAULT		{ 0.1f, 100.0f }
#define P_FOV_DEFAULT		DirectX::XMConvertToRadians(30.0f)

// Orthographic Defaults
#define O_EYE_DEFAULT		{ 0.0f, 0.0f, -1.0f }
#define O_FOCUS_DEFAULT		{ 0.0f, 0.0f, 0.0f }
#define O_CLIP_DEFAULT		{ 0.00f, 1.0f }

namespace BLIB {
	class camera {
		struct constants {
			//float4x4 view;
			float4x4 view_projection;
			float4x4 inverse_view_projection;
			//float4x4 inverse_view;
			//float4x4 inverse_projection;
			float3 camera_position;
			float far_z;
		};
	private:
		mutable constants data;
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer; // Buffer to send Camera data to shaders.

		float2 clip_range;	// { near_z, far_z }
		float3 eye;			// camera position
		float3 focus;		// camera target
		float3 up;

		// Updates the camera's matrices. Automatically defers updating if none is needed.
		void update() const { if (needs_update) { _update(); needs_update = false; } }

	protected:
		mutable matrix V;	// View
		mutable matrix P;	// Projection
		mutable matrix VP;	// View-Projection
		mutable matrix IV;	// Inverse View
		mutable matrix IP;	// Inverse Projection
		mutable matrix IVP; // Inverse View-Projection

		mutable bool needs_update = true;
		// Internal update, for inherited classes to define. Called in update().
		virtual void _update() const = 0;

	public:
		/*
			float2 clip_range
				- initializes near and far z boundary. 
				- float2.x near_z
				- float2.y far_z
			float3 eye
				- initializes camera eye position
			float3 focus
				- initializes the target for the camera to point at.
			float3 up
				- initializes the "up" direction for the camera.
		*/
		camera(float2 clip_range, float3 eye, float3 focus, float3 up = {0, 1, 0});
		virtual ~camera() {}

		float3	get_eye		() const { return eye;				} // get camera position
		float3	get_focus	() const { return focus;			} // get camera target
		float3	get_up		() const { return up;				} // get camera up vector
		float	get_near	() const { return clip_range.x;		} // get camera near z
		float	get_far		() const { return clip_range.y;		} // get camera far z
		float3	get_facing	() const { return focus - eye;		} // get the direction the camera is facing

		void	set_eye		(float3 e)	{ eye = e;			needs_update = true; }
		void	set_focus	(float3 f)	{ focus = f;		needs_update = true; }
		void	set_up		(float3 u)	{ up = u;			needs_update = true; }
		void	set_near	(float d)	{ clip_range.x = d;	needs_update = true; }
		void	set_far		(float d)	{ clip_range.y = d;	needs_update = true; }

		const matrix& get_view()					const { update(); return V;		}
		const matrix& get_projection()				const { update(); return P;		}
		const matrix& get_view_projection()			const { update(); return VP;	}
		const matrix& get_inverse_view()			const { update(); return IV;	}
		const matrix& get_inverse_projection()		const { update(); return IP;	}
		const matrix& get_inverse_view_projection()	const { update(); return IVP;	}

		// Attach the camera buffer to GPU 
		void bind() const;
	};

	// A camera for capturing 3D objects with realistic perspective.
	class perspective_camera : public camera {
	private:
		float aspect_ratio; // Aspect ratio of the viewport of the camera. Width / Height. 
		float fov; // The camera's field of view.

		void _update() const override;
	public:
		perspective_camera(float2 viewport = window::size(), float3 eye = P_EYE_DEFAULT, float3 focus = P_FOCUS_DEFAULT, float fov = P_FOV_DEFAULT, float2 clip_range = P_CLIP_DEFAULT) :
			camera(clip_range, eye, focus), aspect_ratio(viewport.x / viewport.y), fov(fov) {}
		~perspective_camera() {}

		float get_aspect()	const { return aspect_ratio;	}
		float get_fov()		const { return fov;				}

		void set_aspect	(float2 viewport)	{ aspect_ratio = viewport.x / viewport.y;	needs_update = true; }
		void set_fov	(float f)			{ fov = f;									needs_update = true; }

	};

	// A camera for capturing 2D objects, or for capturing 3D objects with no perspective.
	class orthographic_camera : public camera {
	private:
		float2 viewport;

		void _update() const override;
	public:
		orthographic_camera(float2 viewport = window::size(), float3 eye = O_EYE_DEFAULT, float3 focus = O_FOCUS_DEFAULT, float2 clip_range = O_CLIP_DEFAULT) :
			camera(clip_range, eye, focus), viewport(viewport) { }
		~orthographic_camera() {}

		float2 get_viewport() const { return viewport; }

		void set_viewport(float2 v) { viewport = v; needs_update = true; }
	};
}