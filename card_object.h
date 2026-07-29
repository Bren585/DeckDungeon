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

	// マウスオバー
	void select() { selected = true; }
	// マウスオバーではない
	void unselect() { selected = false; }

	//　選択可能
	void activate() { active = true; tint = WHITE; }
	// 選択不可能
	void deactivate() { active = false; tint = { 0.5f, 0.5f, 0.5f }; }
	bool is_active() { return active; }

	void reset() { offset_timer = 0; deactivate(); }

	const card& get_card() { return c; }
};