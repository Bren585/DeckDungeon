#include "title_scene.h"
#include "log.h"
#include "mouse_collider.h"
#include "character_creation_scene.h"
#include "start_dungeon_scene.h"
#include "tutorial_scene.h"

constexpr float add_scale = 0.5f;
constexpr float start_scale = 2.0f;

void title_scene::init() {
	//定義
	const float padding			= BLIB::window::size().x / 16.0f;
	const float quarter_padding	= padding * 0.25f;
	const float font_height		= BLIB::text::height();

	const float left_edge		= BLIB::window::size().x * 0.5f + padding;
	const float right_edge		= BLIB::window::size().x - padding;

	//バックグラウンド
	bkg.load_sprite("ui/bkg.png");
	bkg.set_size(get_size());
	bkg.pos = pos;
	bkg.pivot = pivot;

	//キャラ追加バターン
	add_button.make_dummy(GREEN);
	add_button.pivot = C_TL;
	add_button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, add_button.get_size() * 0.5f));
	add_button.set_size(float2{BLIB::text::width("Add Character"), font_height} * add_scale + float2(quarter_padding));
	add_button.get_collider()->set_off(add_button.pivot * add_button.get_size() * -0.5f);
	
	//スタートバターン
	start_button.make_dummy(WHITE);
	start_button.pos = { left_edge, padding };
	start_button.pivot = C_BL;
	start_button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, start_button.get_size() * 0.5f));
	start_button.set_size({ right_edge - left_edge, font_height * start_scale + padding });
	start_button.get_collider()->set_off(start_button.pivot * start_button.get_size() * -0.5f);

	//チュートリアルバターン
	tutorial_button.make_dummy(WHITE);
	tutorial_button.pos = { left_edge, padding * 1.5f + start_button.get_size().y };
	tutorial_button.pivot = C_BL;
	tutorial_button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, tutorial_button.get_size() * 0.5f));
	tutorial_button.set_size({ right_edge - left_edge, font_height * start_scale * 0.25f + padding });
	tutorial_button.get_collider()->set_off(tutorial_button.pivot * tutorial_button.get_size() * -0.5f);

	//キャラ消すバターン
	delete_button.load_sprite("ui/x.png");
	delete_button.pivot = C_TR;
	delete_button.pos.x = right_edge;
	delete_button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, delete_button.get_size() * 0.5f));
	delete_button.set_size(float2{ font_height });
	delete_button.get_collider()->set_off(delete_button.pivot * delete_button.get_size() * -0.5f);
}

void title_scene::kill() {}

void title_scene::update(float elapsed_time) { 
	// 入力
	auto* mouse = get_mouse_collider();
	bool click = BLIB::input::trigger(key::LClick);

	// 定義
	const float padding			= BLIB::window::size().x / 16.0f;
	const float quarter_padding = padding * 0.25f;
	const float font_height		= BLIB::text::height();

	const float top_edge		= BLIB::window::size().y - padding;
	const float left_edge		= BLIB::window::size().x * 0.5f + padding;
	const float right_edge		= BLIB::window::size().x - padding;

	float2 pos = { left_edge, top_edge };
	pos.y -= font_height + quarter_padding;

	// キャラ消すバターン
	for (auto it = party_data.begin(); it != party_data.end();) {
		delete_button.pos.y = pos.y;
		delete_button.update(0); // コライダーシンク
		pos.y -= font_height * add_scale + quarter_padding;
		if (click) {
			if (BLIB::collision::check(mouse, &delete_button)) { 
				BLIB::audio::play("click");
				it = party_data.erase(it); 
				continue; 
			}
		}
		it++;
	}

	// キャラ追加
	if (party_data.size() < 4) {
		add_button.pos = pos;
		add_button.update(0); // コライダーシンク
		if (BLIB::collision::check(mouse, &add_button)) { // マウスオバー
			if (click) {
				// キャラ作りシーンなければ作成、あれば前のシーンを使います
				if (!cc_id) { cc_id = BLIB::manager::add(new character_creation_scene(get_id())); }
				else { BLIB::manager::get_task(cc_id)->wake(); }
				//シーン移動
				BLIB::manager::stage(cc_id, 0, BLIB::transition::fade, 0.5f);
				BLIB::audio::play("click");
			}
			else add_button.tint = WHITE; 
		}
		else add_button.tint = float3{ 0.8f }; //マウスオバーでない
	}

	// スタート
	start_button.update(0); // コライダーシンク
	if (party_data.size() > 0) { //　キャラいないとスタートできない
		if (BLIB::collision::check(mouse, &start_button)) { //　マウスオバー
			if (click) {
				if (cc_id) { // キャラ作りシーンあれば消す
					static_cast<character_creation_scene*>(BLIB::manager::get_scene(cc_id))->stop();
					cc_id = 0;
				}
				//ダンジョンへ移動
				BLIB::manager::add_and_stage(new start_dungeon_scene(party_data), BLIB::manager::unstage(get_id()), BLIB::transition::fade, 0.5f);
				BLIB::audio::play("click");
				BLIB::audio::stop(bgm_id, 1.0f);
				bgm_id = BLIB::audio::unset;
			}
			else start_button.tint = color(0.5f, 1.0f, 0.5f); //　マウスオバー
		}
		else start_button.tint = color(0.8f, 1.0f, 0.8f); // マウスオバーでないが、キャラが一人以上
	}
	else start_button.tint = color(0.5f, 0.5f, 0.5f); // キャラいない

	// チュートリアル
	tutorial_button.update(0); // コライダーシンク
	if (BLIB::collision::check(mouse, &tutorial_button)) {
		if (click) {
			// シーン移動
			tt_id = BLIB::manager::add_and_stage(new tutorial_scene, 0, BLIB::transition::fade, 0.5f);
			BLIB::audio::play("click");
		}
		else tutorial_button.tint = color(0.5f, 1.0f, 0.5f); //　マウスオバー
	}
	else tutorial_button.tint = color(0.8f, 1.0f, 0.8f); //　マウスオバーでない

}

void title_scene::idle(float elapsed_time) {

}

void title_scene::on_wake() { 
	if (tt_id) { //　チュートリアルから戻ったので、チュートリアルを消します。
		static_cast<tutorial_scene*>(BLIB::manager::get_scene(tt_id))->stop();
		tt_id = 0;
	}
	else if (cc_id) { // キャラ作りから戻ったので、キャラを作成
		party_data.push_back(static_cast<character_creation_scene*>(BLIB::manager::get_scene(cc_id))->get_character_data());
	}
	if (bgm_id == BLIB::audio::unset) { // 音楽なければ出して
		bgm_id = BLIB::audio::play("title", 1.0f, true);
	}
}

void title_scene::draw(BLIB::render_settings rs) const {
	bkg.render(rs); // バックグラウンド

	//　定義
	const float padding				= BLIB::window::size().x / 16.0f;
	const float quarter_padding		= padding * 0.25f;
	const float sixteenth_padding	= padding * 0.0625f;
	const float font_height			= BLIB::text::height();

	const float top_edge			= BLIB::window::size().y - padding;
	const float left_edge			= BLIB::window::size().x * 0.5f + padding;
	const float right_edge			= BLIB::window::size().x - padding;

	// プレイヤー情報
	float2 pos = { left_edge, top_edge };
	pos.y -= type("Party:", pos, float2{ 1 }, FONT_DEFAULT, BLACK, C_TL) + quarter_padding;

	for (auto& data : party_data) {
		delete_button.pos.y = pos.y;
		delete_button.render(rs);
		// 名前
		pos.y -= type(data.name, pos, float2{ add_scale }, FONT_DEFAULT, BLACK, C_TL) + quarter_padding;
		// 消すバターン
		delete_button.update(0);
		//delete_button.peek_collider()->render_debug(rs);
	}

	// 追加バターン
	if (party_data.size() < 4) {
		add_button.render();
		type("Add Character", pos + C_BR * sixteenth_padding, float2(add_scale), FONT_DEFAULT, BLACK, C_TL);
		//add_button.peek_collider()->render_debug(rs);
	}

	// スタートバターン
	start_button.render();
	type("Enter Dungeon", start_button.pos + C_TR * quarter_padding, float2(start_scale), FONT_DEFAULT, BLACK, C_BL);
	//start_button.peek_collider()->render_debug(rs);

	// チュートリアル
	tutorial_button.render();
	type("Tutorial", tutorial_button.pos + C_TR * quarter_padding, float2(start_scale), FONT_DEFAULT, BLACK, C_BL);

	//get_mouse_collider()->peek_collider()->render_debug(rs);

}

