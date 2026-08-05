#pragma once

#include "float3.h"

/***********************************************************************************************************************
													color
************************************************************************************************************************/

#define TEXT_COLOR_TAG '\x1f'
typedef unsigned int hex_rgba;

class color {
public:
	float r, g, b, a;

	color()												: r(1),					g(1),				b(1),				a(1)	{}
	color(float r, float g, float b, float a = 1.0f)	: r(clamp(0, r, 1)),	g(clamp(0, g, 1)),	b(clamp(0, b, 1)),	a(clamp(0, a, 1)) {}
	color(DirectX::XMFLOAT4 c)							: color(c.x, c.y, c.z, c.w)	{}
	color(float3 rgb, float a = 1.0f)					: color(rgb.x, rgb.y, rgb.z, a) {}
	color(hex_rgba hex) : 
		r(((hex >> 24) & 0xff) / 255.0f), 
		g(((hex >> 16) & 0xff) / 255.0f), 
		b(((hex >>  8) & 0xff) / 255.0f), 
		a(((hex		 ) & 0xff) / 255.0f) {}

	inline color operator*(color that) const { return { r * that.r,	g * that.g,	b * that.b, a * that.a }; }

	inline			float& operator[](int i)		{ switch (i) { case 0: return r; case 1: return g; case 2: return b; case 3: return a; default: throw std::out_of_range("Invalid Index"); } }
	inline const	float& operator[](int i) const	{ switch (i) { case 0: return r; case 1: return g; case 2: return b; case 3: return a; default: throw std::out_of_range("Invalid Index"); } }

	inline operator DirectX::XMFLOAT4() const { return { r, g, b, a }; }

	inline operator hex_rgba() const { return 
		((int(r * 255) & 0xff) << 24) |
		((int(g * 255) & 0xff) << 16) |
		((int(b * 255) & 0xff) << 8 ) |
		((int(a * 255) & 0xff)); 
	}

	inline float3 rgb() const { return { r, g, b }; }

	// For some godforsaken reason some APIs expect rgba in reverse order... so here you go.
	inline color abgr() const { return { a, b, g, r }; }

	SERIALIZE(r, g, b, a)
};

// Value To Char : converts a value from a color (i.e., r, g, b, or a) into a char. 
inline char		value_to_char(float v) { return static_cast<			char>(v  * 255 + 0.5f)	; }
inline float	char_to_value(char	c) { return static_cast<unsigned	char>(c) / 255.0f		; }

#define WHITE		color {1, 1, 1}
#define RED			color {1, 0, 0}
#define GREEN		color {0, 1, 0}
#define BLUE		color {0, 0, 1}
#define YELLOW		color {1, 1, 0}
#define AQUA		color {0, 1, 1}
#define PINK		color {1, 0, 1}
#define BLACK		color {0, 0, 0}
#define COLORLESS	color {0, 0, 0, 0}

#define TEXT_COLOR(c) string(TEXT_COLOR_TAG, value_to_char(c.r), value_to_char(c.g), value_to_char(c.b), value_to_char(c.a))