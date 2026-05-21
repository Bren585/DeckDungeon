#include "mouse_collider.h"

BLIB::flat::object mouse;
bool ready = false;

void init() {
	//mouse.set_collider(new BLIB::flat::circle_collider(nullptr, 150));
	mouse.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, float2{ 5 }));
	ready = true;
}

BLIB::flat::object* get_mouse_collider() {
	if (!ready) init();
	mouse.pos = BLIB::input::get_mouse_pos();
	mouse.update(0);
	return &mouse;
}