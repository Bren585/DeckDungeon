#include "character_behavior.h"
#include "character_model.h"

void character_behavior::update(float elapsed_time) {
	switch (state) {
	case state_enter:
		enter();
		state = state_main;
		[[fallthrough]];
	case state_main:
		main(elapsed_time);
		if (condition()) { 
			state = state_exit; 
		}
		break;
	case state_exit:
		exit();
		state = state_finished;
		[[fallthrough]];
	case state_finished:
		break;
	}
}

void character_animation_behavior::enter() {
	parent->get_model()->animate(animation_name, 0.2f, loop);
}

bool character_animation_behavior::condition() {
	return (
		parent->get_model()->remaining_animation_time() < 0.05f
	&&	parent->get_model()->get_current_animation() == animation_name
	||  parent->get_model()->get_current_animation() == NO_ANIMATION
	);
}

bool character_idle_behavior::condition() {
	return parent->get_queue_size() > backlog_max;
}