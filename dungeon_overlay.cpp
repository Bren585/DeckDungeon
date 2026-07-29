#include "dungeon_overlay.h"
#include "dungeon_master.h"
#include "dungeon_scene.h"
#include "mouse_collider.h"
#include "log.h"

//定義

constexpr float PADDING = 10;

//ログボックス

constexpr float LOG_B = 175;
constexpr float LOG_L = 900;
constexpr float LOG_W = 355;
constexpr float LOG_H = 525;

constexpr float LOG_I_B = LOG_B + PADDING;
constexpr float LOG_I_L = LOG_L + PADDING;
constexpr float LOG_I_W = LOG_W - PADDING * 2;
constexpr float LOG_I_H = LOG_H - PADDING * 2;

#define LOG_BL		{LOG_L, LOG_B}
#define LOG_WH		{LOG_W, LOG_H}
#define LOG_I_BL	{LOG_I_L, LOG_I_B}
#define LOG_I_WH	{LOG_I_W, LOG_I_H}

//選択用ボックス

constexpr float CHC_B = 20;
constexpr float CHC_L = 20;
constexpr float CHC_W = LOG_L - (CHC_L + 30);
constexpr float CHC_H = 125;

constexpr float CHC_I_B = CHC_B + PADDING;
constexpr float CHC_I_L = CHC_L + PADDING;
constexpr float CHC_I_W = CHC_W - PADDING * 2;
constexpr float CHC_I_H = CHC_H - PADDING * 2;

#define CHC_BL		{CHC_L, CHC_B}
#define CHC_WH		{CHC_W, CHC_H}
#define CHC_I_BL	{CHC_I_L, CHC_I_B}
#define CHC_I_WH	{CHC_I_W, CHC_I_H}

//キャラ情報ボックス

constexpr float INF_B = CHC_B;
constexpr float INF_L = LOG_L;
constexpr float INF_W = LOG_W;
constexpr float INF_H = CHC_H;

constexpr float INF_I_B = INF_B + PADDING;
constexpr float INF_I_L = INF_L + PADDING;
constexpr float INF_I_W = INF_W - PADDING * 2;
constexpr float INF_I_H = INF_H - PADDING * 2;

#define INF_BL		{INF_L, INF_B}
#define INF_WH		{INF_W, INF_H}
#define INF_I_BL	{INF_I_L, INF_I_B}
#define INF_I_WH	{INF_I_W, INF_I_H}

//ダンジョン画面

const float2 SCN_C = { CHC_L + CHC_W * 0.5f, LOG_B + LOG_H * 0.5f };

// 色

#define BORDER	{ 0.29f, 0.29f, 0.31f }
#define FILL	{ 0.45f, 0.45f, 0.5f }

// カードバック

#define BACK_ID 52

dungeon_master* dungeon_overlay::get_dm() const {
	if (!dm_id) { dm_id = BLIB::manager::find_first_of_type<dungeon_master>(); }
	return GET_DM(dm_id);
}

void dungeon_overlay::init() {
	//シーン設定
	add_settings(BLIB::blend::ALPHA);
	set_background({ 0, 0, 0, 0 });

	//UIを用意
	//フレーム
	BLIB::flat::object frame("ui/overlay.png");
	overlay.draw(&frame);

	//UIのボーダーを書くため資格
	BLIB::flat::object rect;
	rect.make_dummy(WHITE);
	rect.pivot = C_BL;
	rect.tint = BORDER;

	//情報バックスボーダー
	rect.pos = INF_BL;
	rect.set_size(INF_WH);
	overlay.draw(&rect);

	//ログボックスボーダー
	rect.pos = LOG_BL;
	rect.set_size(LOG_WH);
	overlay.draw(&rect);

	//選択ボックスボーダー
	rect.pos = CHC_BL;
	rect.set_size(CHC_WH);
	overlay.draw(&rect);

	//情報UIを用意
	info_canvas.resize(INF_I_WH);
	info_canvas.set_background(FILL);
	info_canvas.pos = INF_I_BL;
	info_canvas.pivot = C_BL;
	info_canvas.clear();

	//ログUIを用意
	log_canvas.resize(LOG_I_WH);
	log_canvas.set_background(FILL);
	log_canvas.pos = LOG_I_BL;
	log_canvas.pivot = C_BL;
	log_canvas.clear();

	//選択UIを用意
	choice_canvas.resize(CHC_I_WH);
	choice_canvas.set_background(FILL);
	choice_canvas.pos = CHC_I_BL;
	choice_canvas.pivot = C_BL;
	choice_canvas.clear();

	//カードスプライトを用意
	for (int suit = 0; suit < max_suit; suit++) {
		for (int face = ace; face < max_face; face++) {
			card_object& obj = card_sprites.emplace_back(card((card_suit)suit, (unsigned int)face));
			obj.get_collider()->set_off(CHC_I_BL);
			obj.pos = float2(-100);
		}
	}
	//カードバック
	card_object& obj = card_sprites.emplace_back(card());
	obj.get_collider()->set_off(obj.get_size() * 0.5f * C_BC + float2 CHC_I_BL);
	obj.pivot = C_TC;
	obj.pos = float2(-100);
}

void dungeon_overlay::update(float elapsed_time) {
	if (overlay.get_size() != BLIB::window::size()) {
		overlay.set_size(BLIB::window::size());
	}

	auto* dm = get_dm();
	if (dm) {
		temp_on_deck = no_show; //情報ボックスをクリア
		BLIB::flat::aligned_rect_collider temp(nullptr, float2(50)); //マウスと当たり判定するため用意
		auto* ds = static_cast<dungeon_scene*>(BLIB::manager::get_scene(BLIB::manager::find_first_of_type<dungeon_scene>()));
		auto* mouse = get_mouse_collider()->peek_collider();

		//プレイヤーをマウスオバーすると、情報をオンデック
		for (auto p : dm->peek_players()) {
			float2 pos = ds->get_character_model(p->get_id()).get_screen_pos(ds->get_camera()) + ds->pos + float2{ 0, 20 };
			temp.sync(pos, float2(1), 0);
			if (BLIB::collision::check(&temp, mouse)) {
				temp_on_deck = p->get_id();
			}
		}

		//敵をマウスオバーすると、情報をオンデック
		for (auto e : dm->peek_enemies()) {
			float2 pos = ds->get_character_model(e->get_id()).get_screen_pos(ds->get_camera()) + ds->pos + float2{ 0, 20 };
			temp.sync(pos, float2(1), 0);
			if (BLIB::collision::check(&temp, mouse)) {
				temp_on_deck = e->get_id();
			}
		}
	}

	//選択アップデート
	update_choice(elapsed_time);

	//アナウンスをアップデート
	if (announcements.size() > 0) {
		if (announcements.front().timer < 0) { announcements.pop(); }
		else { announcements.front().timer -= elapsed_time; }
	}
}

void dungeon_overlay::render_log() const {
	RENDER_LOCK;
	annotate("log");
	const std::vector<string>& messages = event_log::get();
	if (last_log_count != messages.size()) { //	ログ増えてなければ新たに書く必要ない
		last_log_count = messages.size();
		log_canvas.clear();
		//定義
		const float2 font_size = { 8, 16 };
		const float buffer = 5;
		float x = buffer;
		float y = log_canvas.get_size().y - buffer;
		float4 white = {1, 1, 1, 1};
		float4 fade = float4{ FILL, 1 };
		//新しいログを上にし、古ければ古いほど色を薄くする
		for (auto rit = messages.rbegin(); (rit != messages.rend()) && (y > 0); rit++) {
			y -= log_canvas.type(*rit, { x, y }, { 0.5f, 0.5f }, "Arial", lerp(fade, white, y / LOG_I_H), C_TL) + buffer;
		}
	}
}

void dungeon_overlay::render_info() const {
	RENDER_LOCK;
	annotate("info");

	int show = no_show; //リセット
	if (temp_on_deck != no_show) show = temp_on_deck;	//マウスオバーが優先
	else if (on_deck != no_show) show = on_deck;		//後は、アクティブキャラ
	if (show != was_on_deck || force_info) {			//不必要な時に書かない
		force_info = false;
		info_canvas.clear();
		was_on_deck = show;

		if (show != no_show) {
			auto* dm = get_dm();
			const character* info = nullptr;
			for (const auto& character : dm->peek_players()) { if (character->get_id() == show) { info = character; break; } }
			if (info == nullptr) {
				for (const auto& character : dm->peek_enemies()) { if (character->get_id() == show) { info = character; break; } }
			}

			assert(info != nullptr);
			const float half_padding = PADDING * 0.5f;

			/* stats */ {
				float2 pen = { PADDING, INF_I_H - half_padding };
				//名前
				pen.y -= info_canvas.type(info->get_name(), pen, float2{ 0.5f }, "Arial", WHITE, C_TL) + half_padding;

				//値を会得
				int health = info->get_health();
				stats s = info->get_stats();
				stats b = info->get_buffs();

				//出力
				pen.y -= info_canvas.type(string("HP : ",	health, "/", s.health														), pen, float2{ 0.4f }, "Arial", WHITE, C_TL) + half_padding;
				pen.y -= info_canvas.type(string("ATK : ",	s.attack,	(b.attack	? string("(", b.attack	+ b.luck, ")") : string())	), pen, float2{ 0.4f }, "Arial", WHITE, C_TL) + half_padding;
				pen.y -= info_canvas.type(string("DEF : ",	s.defense,	(b.defense	? string("(", b.defense + b.luck, ")") : string())	), pen, float2{ 0.4f }, "Arial", WHITE, C_TL) + half_padding;
				pen.y -= info_canvas.type(string("LCK : ",	s.luck																		), pen, float2{ 0.4f }, "Arial", WHITE, C_TL) + half_padding;
			}

			/* cards */ {
				const pile& hand = info->peek_hand();

				//定義
				const float width = card_sprites[BACK_ID].get_size().x;
				float2 card_position = { INF_I_W - ((PADDING + width) * 2), INF_I_H * 0.5f };

				//カード出力
				for (int i = 0; i < hand.size(); i++) {
					int id = hand[i].id();
					
					float2 old_card_position	= card_sprites[id].pos;
					card_sprites[id].pos		= card_position;
					info_canvas.draw(&card_sprites[id]);
					card_sprites[id].pos		= old_card_position;
					card_position.x += width * 0.75f;
				}
			}
		}
	}
}

void dungeon_overlay::update_choice(float elapsed_time) {
	tooltip = "";
	if (listener) {
		// カード選択
		for (int i = 0; i < visible_cards.size(); i++) {
			card_object* card = visible_cards[i];
			if (listener && card->is_active() && BLIB::collision::check(card, get_mouse_collider())) { 
				//マウスオバー中
				if (card->get_card().suit_is(no_suit))	{ tooltip = "Play the top card of the deck";		}//デック
				else									{ tooltip = string{"Play the ", card->get_card()};	}//手札
				card->select();	
				if (BLIB::input::trigger(key::LClick)) { //選択
					int choice = i;
					if (listener->allow_deck()) choice--; //デック不要ならインデックスが違うため
					listener->on_hear(choice);
					stop_listening();
					BLIB::audio::play("snap");
				}
			}
			else {
				//マウスオバーでない
				card->unselect();
			}

			card->update(elapsed_time);
		}

		// プロンプト選択
		for (int i = 0; i < prompt_buttons.size(); i++) {
			auto* button = &prompt_buttons[i];
			
			if (listener && BLIB::collision::check(button, get_mouse_collider())) {
				//マウスオバー中
				tooltip = choice_prompts[i].desc;
				button->tint = float3{ 0.8f };
				if (BLIB::input::trigger(key::LClick)) { //選択
					listener->on_hear(choice_prompts[i].choice);
					stop_listening();
					BLIB::audio::play("click");
					break;
				}
			}
			else {
				//マウスオバーでない
				button->tint = WHITE;
			}
			button->update(elapsed_time);
		}
	}
}

void dungeon_overlay::render_choice() const {
	RENDER_LOCK;
	choice_canvas.clear();
	if (listener) {
		if (choice_prompts.size() != prompt_buttons.size()) { //プロンプト変化あり
			//定義
			const float2 padding{ PADDING };
			const float scale = 0.5f;
			const float height = BLIB::text::height();
			const float topline = (INF_I_H - PADDING) - (height + padding.y) * (0.5f);
			float column_width = 0;
			float2 pos = float2{ PADDING, topline };

			for (auto prompt : choice_prompts) {
				float2 size = float2{ BLIB::text::width(prompt.name) + PADDING, height } *scale;
				if (size.x > column_width) { column_width = size.x; }
				//バターン作成
				canvas& button = prompt_buttons.emplace_back(size + padding);
				button.set_background(BORDER);
				button.clear();
				button.type(prompt.name, padding * 0.5f, float2{ scale }); //プロンプト名をバターンに書く
				button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, button.get_size() * 0.5f));
				button.get_collider()->set_off((button.get_size() * 0.5f) * C_BR + float2 CHC_I_BL);
				button.pivot = C_TL;
				button.pos = pos;
				pos.y -= height + PADDING;
				if (pos.y - height < 0) { // 下に行き過ぎない
					pos.y = topline;
					pos.x += column_width + PADDING * 2;
					column_width = 0;
				}
			}
		}

		//　選択のメッセージ
		choice_canvas.type(listener->title(), { PADDING * 0.5f, CHC_I_H - PADDING * 0.5f }, float2{ 0.5f }, "Arial", WHITE, C_TL);

		//　手札
		for (int i = 0; i < visible_cards.size(); i++) {
			choice_canvas.draw(visible_cards[i]);
		}

		// バターン
		for (int i = 0; i < prompt_buttons.size(); i++) {
			choice_canvas.draw(&prompt_buttons[i]);
		}
	}
}

void dungeon_overlay::draw(BLIB::render_settings rs) const {
	//　フレーム
	overlay.render(rs);

	//　ログ
	render_log();
	log_canvas.render(rs);

	//　情報
	render_info();
	info_canvas.render(rs);

	// 選択
	render_choice();
	choice_canvas.render(rs);

	// ツールチップ
	if (tooltip != "") {
		type(tooltip, BLIB::input::get_mouse_pos() - float2(1), float2{ 0.5f }, FONT_DEFAULT, BLACK);
		type(tooltip, BLIB::input::get_mouse_pos(),				float2{ 0.5f });
	}

	//　アナウンス
	if (announcements.size() > 0) {
		announcement front = announcements.front();
		float a = front.timer / front.max;
		a = 1 - powf(2 * (a - 0.5f), 8);
		type(front.message, SCN_C - float2{2},	float2(4), FONT_DEFAULT, color(0, 0, 0, a), C_CC);
		type(front.message, SCN_C,				float2(4), FONT_DEFAULT, color(1, 1, 1, a), C_CC);
	}

	////デバッグ
	//for (auto& b : prompt_buttons) {
	//	b.peek_collider()->render_debug(rs);
	//}
	//
	//for (auto& c : visible_cards) {
	//	c->peek_collider()->render_debug(rs);
	//}
	//
	//get_mouse_collider()->peek_collider()->render_debug(rs);
	//type(get_mouse_collider()->pos, float2 { PADDING }, float2{ 1 }, "Arial", WHITE, C_BL);
}

void dungeon_overlay::start_listening(choice_listener* l) { 
	RENDER_LOCK;

	//選択の情報を会得
	listener = l; 
	choice_prompts = l->get_prompts(); 
	if (listener->show_info() != no_show) {
		on_deck = listener->show_info();
	}

	//クリア
	prompt_buttons.clear();
	visible_cards.clear();

	if (l->show_info() != no_show) {
		force_info = true; //情報をアップデートさせる

		const float width = card_sprites[BACK_ID].get_size().x;
		float2 card_position = { CHC_I_W * 0.5f - ((PADDING + width) * 1.5f), CHC_I_H * 0.25f };

		if (l->allow_deck()) { // 山から引いていいので、山を用意
			card_sprites[BACK_ID].set_rest_pos(card_position);
			card_sprites[BACK_ID].reset();
			card_sprites[BACK_ID].activate();
			visible_cards.push_back(&card_sprites[BACK_ID]);
		}
		card_position.x += PADDING + width;

		if (l->allow_cards()) { // 手札から出してもいいので、手札を用意
			//キャラ会得
			int show = l->show_info();
			auto* dm = get_dm();
			const character* info = nullptr;
			for (const auto& character : dm->peek_players()) { if (character->get_id() == show) { info = character; break; } }
			if (info == nullptr) {
				for (const auto& character : dm->peek_enemies()) { if (character->get_id() == show) { info = character; break; } }
			}
			assert(info);

			//　手札会得
			const pile& hand = info->peek_hand();

			for (int i = 0; i < hand.size(); i++) {
				//　カード用意
				int id = hand[i].id();
				card_sprites[id].set_rest_pos(card_position);
				card_sprites[id].reset();
				visible_cards.push_back(&card_sprites[id]);
				//　カードが許可されればオンにする
				if (listener->allow_suit() == any_suit || listener->allow_suit() == card_sprites[id].get_card().suit()) {
					card_sprites[id].activate();
				}
				card_position.x += PADDING + width;
			}
		}
	}
}