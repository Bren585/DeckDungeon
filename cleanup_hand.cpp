#include "cleanup_hand.h"
#include "dungeon_overlay.h"
#include "dungeon_scene.h"

void cleanup_hand::init() {
	if (player->get_hand().size() == 0) {
		discarded = true;
		int card_count = (int)player->get_hand().size();
		int draw_count = 3 /* max hand size */ - card_count;
		player->get_deck().draw(draw_count).into(player->get_hand());
	}
}

void cleanup_hand::on_wake() { 
	get_overlay()->start_listening(this); 
	get_dungeon_scene()->spotlight(player->get_id(), GREEN);
}

void cleanup_hand::end_cleanup() {
	int card_count = (int)player->get_hand().size();
	int draw_count = 3 /* max hand size */ - card_count;
	player->get_deck().draw(draw_count).into(player->get_hand());

	if (discarded) {
		mulligan.into(player->get_deck());
		player->get_deck().shuffle();
		finish();
	}
	else {
		discarded = true;
		sleep();
	}

	get_dungeon_scene()->clear_spotlight();
}

void cleanup_hand::on_hear(int choice) {
	if (choice == -1) {			
		end_cleanup();
	}
	else {
		player->get_hand().play(choice).into(mulligan);
		if (player->get_hand().size() == 0) { end_cleanup(); }
		else { sleep(); }
	}
}

void cleanup_hand::on_stop() { get_overlay()->stop_listening(); }