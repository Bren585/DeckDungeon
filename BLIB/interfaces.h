#pragma once
#include "render_settings.h"

namespace BLIB {
	// A class with the render(render_settings) function signature.
	class renderable {
	public:
		virtual void render(render_settings) const = 0;
		virtual ~renderable() = default;
	};

	// A class with the update(float) function signature.
	class updateable {
	public:
		virtual void update(float) = 0;
		virtual ~updateable() = default;
	};
}