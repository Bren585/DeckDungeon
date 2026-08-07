#include "dungeon_master.h"
#include "dungeon_scene.h"
#include "dungeon_overlay.h"
#include "title_scene.h"
#include "log.h"
#include "BLIB\transition_scene.h"

#include "begin_floor.h"
#include "combat.h"
#include "end_floor.h"
#include "boss_floor_flag.h"

std::vector<character*>& DM_priviledges::get_players() { return GET_DM(dm_id)->players; }
std::vector<character*>& DM_priviledges::get_enemies() { return GET_DM(dm_id)->enemies; }

dungeon_scene* DM_animator::get_scene() { return static_cast<dungeon_scene*>(BLIB::manager::get_scene(scene_id)); }

constexpr int dungeon_start		= 0;
constexpr int dungeon_survive	= 1;
constexpr int dungeon_dead		= 2;
constexpr int dungeon_end		= 3;

void dungeon_master::update(float elapsed_time) {
	BEGIN_COROUTINE;

	YIELD_WHILE(BLIB::manager::get_scene(BLIB::manager::find_first_of_type<dungeon_scene>())->report() != active);
	
	SET_CHECKPOINT(dungeon_start);

	//　音楽切り替え
	if (rest_bgm != BLIB::audio::unset) {
		BLIB::audio::stop(rest_bgm, 1.0f);
		rest_bgm = BLIB::audio::unset;
	}
	combat_bgm = BLIB::audio::play("battle", 1.0f, true);

	WAIT(2.0f);

	// フロア作成
	// 敵作成
	// プレイヤー手札片付け
	YIELD_SUBTASK(begin_floor, floor_number);

	event_log::record("Finished floor generation");
	get_overlay()->announce("Combat Start!");

	// クリアもしくは負けまで
	YIELD_SUBTASK(combat);

	//　音楽を止まる
	if (combat_bgm != BLIB::audio::unset) {
		BLIB::audio::stop(combat_bgm, 1.0f);
		combat_bgm = BLIB::audio::unset;
	}

	{
		//　クリアか負けかチェック
		bool survived = false;
		for (auto& player : players) { if (player->alive()) { survived = true; break; } }

		if (survived) {
			GO_TO_CHECKPOINT(dungeon_survive);
		}
		else {
			GO_TO_CHECKPOINT(dungeon_dead);
		}
	}

	SET_CHECKPOINT(dungeon_survive); // クリア

	// 音楽切り替え
	rest_bgm = BLIB::audio::play("rest", 1.0f, true);

	// 喜びアニメーション
	get_overlay()->announce("Floor Clear!");
	event_log::record(string("Floor ", floor_number, " cleared!"));
	for (auto& player : players) {
		if (player->alive()) {
			get_scene()->get_character_model(player->get_id()).start_behavior<cb::celebrate>();
		}
	}
	WAIT(1.0f);

	// フロアクリア
	// 復活
	// レベルアップ
	// 死亡者消す
	get_overlay()->announce("Level Up!");
	YIELD_SUBTASK(end_floor);

	// 画面一回暗くする
	BLIB::manager::unstage(scene_id, BLIB::transition::fade, 1.0f);
	YIELD_WHILE(BLIB::manager::find_first_of_type<BLIB::transition_scene>());
	BLIB::manager::stage(scene_id, 1, BLIB::transition::fade, 1.0f);

	// 次のフロア
	floor_number++;
	GO_TO_CHECKPOINT(dungeon_start);

	SET_CHECKPOINT(dungeon_dead); // ゲームオーバー

	// 音楽
	BLIB::audio::play("tpk", 1.0f, true);

	// アナウンス
	get_overlay()->announce("Game Over!");
	event_log::record("Game over");
	event_log::record(string("Your party survived ", floor_number, " floors"));

	YIELD_WHILE(!BLIB::input::trigger());

	SET_CHECKPOINT(dungeon_end);

	// シーン片付け
	BLIB::manager::unstage(scene_id, BLIB::transition::fade, 1.0f);
	BLIB::manager::unstage(BLIB::manager::find_first_of_type<dungeon_overlay>(), BLIB::transition::fade, 1.0f);
	get_scene()->stop();
	get_overlay()->stop();

	// シーン切り替え終わりまで待つ
	YIELD_WHILE(BLIB::manager::find_first_of_type<BLIB::transition_scene>());

	//音楽消す
	BLIB::audio::stop(BLIB::audio::all_tracks, 1.0f);

	//タイトル画面へ
	BLIB::manager::stage(BLIB::manager::find_first_of_type<title_scene>(), 0, BLIB::transition::fade, 0.5f);
	finish();

	END_COROUTINE;
}

void dungeon_master::kill() { 
	for (auto& player : players) { delete player; } 
	for (auto& enemy : enemies) { delete enemy; } 
}

void dungeon_master::on_stop() {
	BLIB::task_id task = BLIB::manager::find_first_of_type<begin_floor>();
	if (task) { BLIB::manager::get_task(task)->stop(); }

	task = BLIB::manager::find_first_of_type<combat>(); 
	if (task) { BLIB::manager::get_task(task)->stop(); }

	task = BLIB::manager::find_first_of_type<end_floor>();
	if (task) { BLIB::manager::get_task(task)->stop(); }

	task = BLIB::manager::find_first_of_type<boss_floor_flag>();
	if (task) { BLIB::manager::get_task(task)->stop(); }

	GO_TO_CHECKPOINT(dungeon_end);
}