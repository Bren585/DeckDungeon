#pragma once
#include "math.h"

typedef unsigned long long keymask;

namespace key {
#define KEY_START_LINE 9
#define KEY(name) constexpr keymask name = (1ULL << (__LINE__ - KEY_START_LINE))
	// This is the start line
	KEY(Up);
	KEY(Down);
	KEY(Left);
	KEY(Right);
	KEY(Enter);
	KEY(Back);
	KEY(Space);
	KEY(Z);
	KEY(X);
	KEY(I);
	KEY(J);
	KEY(K);
	KEY(L);
	KEY(Esc);
	KEY(LClick);
	KEY(RClick);
	KEY(MClick);
	KEY(LShift);
	KEY(RShift);
	KEY(LAlt);
	KEY(RAlt);
	KEY(LControl);
	KEY(RControl);
	KEY(Backspace);

	constexpr keymask Shift		= LShift	| RShift;
	constexpr keymask Alt		= LAlt		| RAlt;
	constexpr keymask Control	= LControl	| RControl;

#undef KEY
}

namespace BLIB {
	
	namespace input {

		// Setup input devices.
		void init();
		// Detect input from devices.
		void update();
		// Handle text input.
		void wm_char(wchar_t c);

		// Returns true if the mouse is locked.
		bool mouse_locked();
		// Lock or unlock the mouse. Locked mouse is good for first person applications.
		void set_mouse_locked(bool locked);

		// Get the position of the mouse. In locked mode, get the movement of the mouse.
		float2 get_mouse_pos();
		// Get the distance the scroll wheel has scrolled this frame.
		float get_mouse_scroll();
		// Get the current position of the scroll wheel.
		float get_mouse_scroll_value();
		// Schedule the scroll wheel position to be reset.
		void reset_mouse_scroll_value();

		// The current state of all keys (Whether they are being pressed or not)
		keymask state();
		// Returns all keys newly pressed on this frame.
		keymask trigger();
		// Returns all keys that stopped being pressed on this frame.
		keymask release();

		// Get the current state of a key.
		inline bool state	(keymask key) {return key & state	();}
		// Checl if a key was pressed this frame.
		inline bool trigger	(keymask key) {return key & trigger	();}
		// Check if a key was released on this frame.
		inline bool release	(keymask key) {return key & release	();}

		// Get the buffer text is being typed to.
		string& get_typing_buffer();

	}

}
