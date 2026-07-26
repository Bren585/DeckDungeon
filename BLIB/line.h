#pragma once
#include "shape.h"

namespace BLIB::debug::draw {
	/*
		float3 start
			- Origin of the line.
		float3 end
			- Endpoint of the line.
		color c
			- The color of the line.
	*/
	inline void line(float3 start, float3 end, color c) {
		constexpr unsigned int vertex_count = 2;
		vertex vertices[vertex_count] = { {start, c}, {end, c} };
		push_to_screen(vertices, vertex_count);
	}

	/*
		float2 start
			- Origin of the line.
		float2 end
			- Endpoint of the line.
		color c
			- The color of the line.
	*/
	inline void line(float2 start, float2 end, color c) {
		line({ start, 0 }, { end, 0 }, c);
	}
}