#include "character.h"
#include "dungeon_scene.h"
int character::id_counter = 0;

character::character(string name, stats s, character_flags f) : 
	id(id_counter++),
	name(name), 
	base(s), 
	health(s.health),
	flags(f)
{

}

void character::make_model(string model, string alt) {
	BLIB::task_id dungeon_scene_id = BLIB::manager::find_first_of_type<dungeon_scene>();
	assert(dungeon_scene_id);
	static_cast<dungeon_scene*>(BLIB::manager::get_scene(dungeon_scene_id))->register_character_model(id, model, alt, this);
}