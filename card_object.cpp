#include "card_object.h"
#include "mouse_collider.h"

constexpr float offset_amount = 10;
constexpr float offset_duration = 0.15f;

card_object::card_object(card c) : BLIB::flat::object(c.sprite_name()), c(c) {
	set_collider(new BLIB::flat::aligned_rect_collider(nullptr, get_size() * 0.5f));
	scale = float2{ 0.25f };
	pivot = C_CC;
}

void card_object::update(float elapsed_time) {
	if (selected && offset_timer < offset_duration)	{ offset_timer += elapsed_time; if (offset_timer > offset_duration	) offset_timer = offset_duration;	}
	else if (!selected && offset_timer > 0)			{ offset_timer -= elapsed_time; if (offset_timer < 0				) offset_timer = 0;					}

	pos = true_pos;
	if (offset_timer > 0) {
		pos.y += offset_amount * (offset_timer / offset_duration);
	}

	object::update(elapsed_time);
}