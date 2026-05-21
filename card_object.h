#pragma once
#include "BLIB\object.h"
#include "cards.h"

class card_object : public BLIB::flat::object {
	bool selected = false;
	bool active = false;
	float2 true_pos;
	float offset_timer = 0;
	card c;
public:
	card_object(card c);

	void update(float elapsed_time);

	void set_rest_pos(float2 pos) { true_pos = pos; }

	void select() { selected = true; }
	void unselect() { selected = false; }

	void activate() { active = true; tint = WHITE; }
	void deactivate() { active = false; tint = { 0.5f, 0.5f, 0.5f }; }
	bool is_active() { return active; }

	void reset() { offset_timer = 0; deactivate(); }

	card_suit suit() { return c.suit(); }
};