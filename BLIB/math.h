#pragma once

// Somehow including just triangle gets every piece of the math library... so.
#include "triangle.h"

/*
	My custom math library includes the following:

	float2
	float3
	float4
	float3x3	// Used for normalized rotation matrices
	float4x4	// just a wrapper for DirectXMath::FLOAT4X4 + tools 
	matrix		// just a wrapper for DirectXMath::MATRIX + tools
	quaternion	// A float4 with tools for quaternion rotations
	color		// A float4 with with r, g, b, a instead of x, y, z, w + tools
	transform	// A representation of a 3D transformation, split into parts (position, translation, rotation, scale) with matrix support
	triangle	// A collection of 3D points, mostly here for hit detection and raycast support
	
	The floatx classes have the following built-in:
	
	operator+(floatx)
	operator-(floatx)
	operator*(float)	// scalar muliplication
	operator*(floatx)	// vector multiplication 
	operator/(float)	// scalar division
	operator/(floatx)	// vector division
	corresponding = operators (i.e., +=) for the above
	operator%(floatx)	// Cross product.
	comparison operators (==, <, >, etc)
	mag_sq()			// Returns the squared magnitude of the vector
	mag()				// Returns the magnitude of the vector
	norm()				// Returns a normalized unit vector
	floor()				// Rounds the vector up to the nearest integer coordinates
	ceil()				// Rounds the vector down to the nearest integer coordinates
	abs()				// Returns the vector, but all of the components have been made positive.

	The following functions have been prepared for most datatypes (including int, float, and floatx):

	maxim(a, b)			// Returns the greater of the two values
	minim(a, b)			// Returns the smaller of the two values
	clamp(min, x, max)	// Clamps x to the given bounds
	wrap(min, x, max)	// Wraps x within the given bounds
	lerp(a, b, t)		// Linearly interpolates the given values on t.
	is_zero(x)			// Returns true if x is approximately zero.
	non_zero(x)			// Returns true if x is NOT approximately zero.

	And, for floatx types:

	dot(floatx, floatx)		// Calculates the dot product.
	dist(floatx, floatx)	// Returns the distance between the two vectors.

	floatx types can be constructed by smaller floatx types, for instance:

	float4(float2, float2)

	floatx types are also decomposable, using either any combination of their members,

	float4().xz() -> float2

	or by zeroing their memebers,

	float3(1, 2, 3).xoz() -> { 1, 0, 3 }

*/