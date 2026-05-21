#include <time.h>

#include "BLIB/BLIB.h"
using namespace BLIB;

#ifdef _DEBUG
#include "BLIB/device.h"
#endif

#include "BLIB/load_scene.h"
#include "all_scenes.h" 
#include "model_test.h"

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_  HINSTANCE prev_instance, _In_ LPSTR cmd_line, _In_ int cmd_show)
{
	srand(static_cast<unsigned int>(time(nullptr)));

	text::	set_filepath(L"data/fonts/");
	audio::	set_filepath(L"data/audio/");
	sprite::set_filepath(L"data/images/");
	model::	set_filepath(L"data/models/");
	shader::set_filepath(L"data/shaders/");

	if (!init(instance, prev_instance, cmd_line, cmd_show, L"Deck Dungeon")) { return 0; }

#ifdef _DEBUG
	device::debug(); // Enable Debug Layer
#endif

	//load_scene_any::set_background(L"background.png");
	//load_scene_any::set_background(BLACK);
	load_scene_any::set_load_icon(L"ui/loading.png"); 
	//load_scene_any::set_icon_spin_speed(PI2);
	//load_scene_any::set_test("Loading...");
	//load_scene_any::set_text_font(FONT_DEFAULT);
	//load_scene_any::set_text_color(WHITE); 

	manager::add_and_stage(new title_scene(), 0);

	while (loop());

#ifdef _DEBUG
	//device::report_live();
#endif
	
	uninit();
	
	return 0;
}