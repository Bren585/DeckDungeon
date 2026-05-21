#include "cleanup_hand.h"
#include "dungeon_overlay.h"

void cleanup_hand::init() {
	if (player->get_hand().size() == 0) {
		discarded = true;
		int card_count = (int)player->get_hand().size();
		int draw_count = 3 /* max hand size */ - card_count;
		player->get_deck().draw(draw_count).into(player->get_hand());
	}
}

void cleanup_hand::on_wake() { get_overlay()->start_listening(this); }

//#include "BLIB\imgui\imgui.h"
//void cleanup_hand::update(float elapsed_time) {
//	string message = discarded ? "mulligan your hand" : "discard unwanted cards";
//	string title(string(player->get_name(), " you may ", message));
//	if (ImGui::Begin(title)) {
//		int i = 0;
//		for (auto& card : player->get_hand()) {
//			string label("Discard ", string(card), "?");
//			ImGui::Checkbox(label, &discard[i]);
//			i++;
//		}
//		if (ImGui::Button("Confirm")) {
//			for (int i = 2; i > -1; i--) {
//				if (discard[i]) {
//					player->get_hand().play(i).into(mulligan);
//					discard[i] = false;
//				}
//			}
//
//			int card_count = (int)player->get_hand().size();
//			int draw_count = 3 /* max hand size */ - card_count;
//			player->get_deck().draw(draw_count).into(player->get_hand());
//
//			if (discarded) {
//				mulligan.into(player->get_deck());
//				player->get_deck().shuffle();
//				finish();
//			}
//			else discarded = true;
//		}
//	}
//	ImGui::End();
//}

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