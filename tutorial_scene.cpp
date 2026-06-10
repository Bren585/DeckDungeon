#include "tutorial_scene.h"
#include "BLIB\input.h"
#include "title_scene.h"

void tutorial_scene::init() {
	bkg.load_sprite("tutorial.png");
	bkg.pos = pos;
	bkg.pivot = pivot;
	bkg.set_size(get_size());
}

void tutorial_scene::update(float elapsed_time) {
	if (timer > 1.5f && BLIB::input::trigger()) {
		finish();
		BLIB::manager::stage(BLIB::manager::find_first_of_type<title_scene>(), 0, BLIB::transition::fade, 0.5f);
	}
}
