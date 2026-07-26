#pragma once
#include "math.h"

namespace BLIB::debug::draw {
	struct vertex {
		float3 pos;
		color color;
	};

	// Draw a polygon with the given vertices. 
	// If the buffer is uninitialized, it will be automatically initialized.
	void push_to_screen(vertex vertices[], unsigned int vertex_count);
	// Empty debug::draw buffers.
	void uninit();
}