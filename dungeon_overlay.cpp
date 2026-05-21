#include "dungeon_overlay.h"
#include "dungeon_master.h"
#include "dungeon_scene.h"
#include "mouse_collider.h"
#include "log.h"

constexpr float PADDING = 10;

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

#define BORDER	{ 0.29f, 0.29f, 0.31f }
#define FILL	{ 0.45f, 0.45f, 0.5f }

#define BACK_ID 52

dungeon_master* dungeon_overlay::get_dm() const {
	if (!dm_id) { dm_id = BLIB::manager::find_first_of_type<dungeon_master>(); }
	return GET_DM(dm_id);
}

void dungeon_overlay::init() {
	add_settings(BLIB::blend::ALPHA);
	set_background({ 0, 0, 0, 0 });

	BLIB::flat::object frame("ui/overlay.png");
	overlay.draw(&frame);

	BLIB::flat::object rect;
	rect.make_dummy(WHITE);
	rect.pivot = C_BL;

	rect.tint = BORDER;

	rect.pos = INF_BL;
	rect.set_size(INF_WH);
	overlay.draw(&rect);

	rect.pos = LOG_BL;
	rect.set_size(LOG_WH);
	overlay.draw(&rect);

	rect.pos = CHC_BL;
	rect.set_size(CHC_WH);
	overlay.draw(&rect);

	info_canvas.resize(INF_I_WH);
	info_canvas.set_background(FILL);
	info_canvas.pos = INF_I_BL;
	info_canvas.pivot = C_BL;
	info_canvas.clear();

	log_canvas.resize(LOG_I_WH);
	log_canvas.set_background(FILL);
	log_canvas.pos = LOG_I_BL;
	log_canvas.pivot = C_BL;
	log_canvas.clear();

	choice_canvas.resize(CHC_I_WH);
	choice_canvas.set_background(FILL);
	choice_canvas.pos = CHC_I_BL;
	choice_canvas.pivot = C_BL;
	choice_canvas.clear();

	for (int suit = 0; suit < max_suit; suit++) {
		for (int face = ace; face < max_face; face++) {
			card_object& obj = card_sprites.emplace_back(card((card_suit)suit, (unsigned int)face));
			obj.get_collider()->set_off(CHC_I_BL);
			obj.pos = float2(-100);
		}
	}
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
		temp_on_deck = no_show;
		BLIB::flat::aligned_rect_collider temp(nullptr, float2(50));
		auto* ds = static_cast<dungeon_scene*>(BLIB::manager::get_scene(BLIB::manager::find_first_of_type<dungeon_scene>()));
		auto* mouse = get_mouse_collider()->peek_collider();

		for (auto p : dm->peek_players()) {
			float2 pos = ds->get_character_model(p->get_id()).get_screen_pos(ds->get_camera()) + ds->pos + float2{ 0, 20 };
			temp.sync(pos, float2(1), 0);
			if (BLIB::collision::check(&temp, mouse)) {
				temp_on_deck = p->get_id();
			}
		}

		for (auto e : dm->peek_enemies()) {
			float2 pos = ds->get_character_model(e->get_id()).get_screen_pos(ds->get_camera()) + ds->pos + float2{ 0, 20 };
			temp.sync(pos, float2(1), 0);
			if (BLIB::collision::check(&temp, mouse)) {
				temp_on_deck = e->get_id();
			}
		}
	}

	update_choice(elapsed_time);
}

void dungeon_overlay::render_log() const {
	RENDER_LOCK;
	annotate("log");
	const std::vector<string>& messages = event_log::get();
	if (last_log_count != messages.size()) {
		last_log_count = messages.size();
		log_canvas.clear();
		const float2 font_size = { 8, 16 };
		const float buffer = 5;
		float x = buffer;
		float y = log_canvas.get_size().y - buffer;
		float4 white = {1, 1, 1, 1};
		float4 fade = float4{ FILL, 1 };
		for (auto rit = messages.rbegin(); (rit != messages.rend()) && (y > 0); rit++) {
			y -= log_canvas.type(*rit, { x, y }, { 0.5f, 0.5f }, "Arial", lerp(fade, white, y / LOG_I_H), C_TL) + buffer;
		}
	}
}

void dungeon_overlay::render_info() const {
	RENDER_LOCK;
	annotate("info");

	int show = no_show;
	if (temp_on_deck != no_show) show = temp_on_deck;
	else if (on_deck != no_show) show = on_deck;
	if (show != was_on_deck || force_info) {
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
				pen.y -= info_canvas.type(info->get_name(), pen, float2{ 0.5f }, "Arial", WHITE, C_TL) + half_padding;

				int health = info->get_health();
				stats s = info->get_stats();
				stats b = info->get_buffs();

				pen.y -= info_canvas.type(string("HP : ",	health, "/", s.health														), pen, float2{ 0.4f }, "Arial", WHITE, C_TL) + half_padding;
				pen.y -= info_canvas.type(string("ATK : ",	s.attack,	(b.attack	? string("(", b.attack	+ b.luck, ")") : string())	), pen, float2{ 0.4f }, "Arial", WHITE, C_TL) + half_padding;
				pen.y -= info_canvas.type(string("DEF : ",	s.defense,	(b.defense	? string("(", b.defense + b.luck, ")") : string())	), pen, float2{ 0.4f }, "Arial", WHITE, C_TL) + half_padding;
				pen.y -= info_canvas.type(string("LCK : ",	s.luck																		), pen, float2{ 0.4f }, "Arial", WHITE, C_TL) + half_padding;
			}

			/* cards */ {
				const pile& hand = info->peek_hand();

				//const float scale = 0.25f;
				const float width = card_sprites[BACK_ID].get_size().x;// * scale;

				float2 card_position = { INF_I_W - ((PADDING + width) * 2), INF_I_H * 0.5f };

				for (int i = 0; i < hand.size(); i++) {
					int id = hand[i].id();
					
					float2 old_card_position	= card_sprites[id].pos;
					//float2 old_scale			= card_sprites[id].scale;
					card_sprites[id].pos		= card_position;
					//card_sprites[id].scale	= float2{ scale };
					info_canvas.draw(&card_sprites[id]);
					card_sprites[id].pos		= old_card_position;
					//card_sprites[id].scale	= old_scale;
					card_position.x += width * 0.75f;
				}
			}
		}
	}
}

void dungeon_overlay::update_choice(float elapsed_time) {
	tooltip = "";
	if (listener) {
		// update cards
		for (int i = 0; i < visible_cards.size(); i++) {
			card_object* card = visible_cards[i];
			if (listener && card->is_active() && BLIB::collision::check(card, get_mouse_collider())) { 
				card->select();	
				if (BLIB::input::trigger(key::LClick)) {
					int choice = i;
					if (listener->allow_deck()) choice--;
					listener->on_hear(choice);
					stop_listening();
					BLIB::audio::play("snap");
				}
			}
			else {
				card->unselect();
			}

			card->update(elapsed_time);
			//choice_canvas.draw(card);
		}

		// prompts
		for (int i = 0; i < prompt_buttons.size(); i++) {
			auto* button = &prompt_buttons[i];
			
			if (listener && BLIB::collision::check(button, get_mouse_collider())) {
				tooltip = choice_prompts[i].desc;
				button->tint = float3{ 0.8f };
				if (BLIB::input::trigger(key::LClick)) {
					listener->on_hear(choice_prompts[i].choice);
					stop_listening();
					BLIB::audio::play("click");
					break;
				}
			}
			else {
				button->tint = WHITE;
			}
			button->update(elapsed_time);
			//choice_canvas.draw(button);
		}
	}
}

void dungeon_overlay::render_choice() const {
	RENDER_LOCK;
	choice_canvas.clear();
	if (listener) {
		if (choice_prompts.size() != prompt_buttons.size()) {
			const float2 padding{ PADDING };
			const float scale = 0.5f;
			const float height = BLIB::text::height();
			const float topline = (INF_I_H - PADDING) - (height + padding.y) * (0.5f);
			float column_width = 0;
			float2 pos = float2{ PADDING, topline };
			for (auto prompt : choice_prompts) {
				float2 size = float2{ BLIB::text::width(prompt.name) + PADDING, height } *scale;
				if (size.x > column_width) { column_width = size.x; }
				canvas& button = prompt_buttons.emplace_back(size + padding);
				button.set_background(BORDER);
				button.clear();
				button.type(prompt.name, padding * 0.5f, float2{ scale });
				button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, button.get_size() * 0.5f));
				button.get_collider()->set_off((button.get_size() * 0.5f) * C_BR + float2 CHC_I_BL);
				button.pivot = C_TL;
				button.pos = pos;
				pos.y -= height + PADDING;
				if (pos.y - height < 0) {
					pos.y = topline;
					pos.x += column_width + PADDING * 2;
					column_width = 0;
				}
			}
		}

		choice_canvas.type(listener->title(), { PADDING * 0.5f, CHC_I_H - PADDING * 0.5f }, float2{ 0.5f }, "Arial", WHITE, C_TL);

		for (int i = 0; i < visible_cards.size(); i++) {
			choice_canvas.draw(visible_cards[i]);
		}

		// prompts
		for (int i = 0; i < prompt_buttons.size(); i++) {
			choice_canvas.draw(&prompt_buttons[i]);
		}
	}
}

void dungeon_overlay::draw(BLIB::render_settings rs) const {
	overlay.render(rs);

	render_log();
	log_canvas.render(rs);
	render_info();
	info_canvas.render(rs);
	render_choice();
	choice_canvas.render(rs);

	if (tooltip != "") {
		type(tooltip, BLIB::input::get_mouse_pos(), float2{0.5f});
	}
	
	//for (auto& b : prompt_buttons) {
	//	b.peek_collider()->render_debug(rs);
	//}

	//for (auto& c : visible_cards) {
	//	c->peek_collider()->render_debug(rs);
	//}
	
	//get_mouse_collider()->peek_collider()->render_debug(rs);
	//type(get_mouse_collider()->pos, float2 { PADDING }, float2{ 1 }, "Arial", WHITE, C_BL);
}

void dungeon_overlay::start_listening(choice_listener* l) { 
	RENDER_LOCK;

	listener = l; 
	choice_prompts = l->get_prompts(); 
	if (listener->show_info() != no_show) {
		on_deck = listener->show_info();
	}

	prompt_buttons.clear();
	visible_cards.clear();

	if (l->show_info() != no_show) {
		force_info = true;

		const float width = card_sprites[BACK_ID].get_size().x;
		float2 card_position = { CHC_I_W * 0.5f - ((PADDING + width) * 1.5f), CHC_I_H * 0.25f };

		if (l->allow_deck()) {
			card_sprites[BACK_ID].set_rest_pos(card_position);
			card_sprites[BACK_ID].reset();
			card_sprites[BACK_ID].activate();
			visible_cards.push_back(&card_sprites[BACK_ID]);
		}
		card_position.x += PADDING + width;

		if (l->allow_cards()) {
			int show = l->show_info();
			auto* dm = get_dm();
			const character* info = nullptr;
			for (const auto& character : dm->peek_players()) { if (character->get_id() == show) { info = character; break; } }
			if (info == nullptr) {
				for (const auto& character : dm->peek_enemies()) { if (character->get_id() == show) { info = character; break; } }
			}
			assert(info);

			const pile& hand = info->peek_hand();

			for (int i = 0; i < hand.size(); i++) {
				int id = hand[i].id();
				card_sprites[id].set_rest_pos(card_position);
				card_sprites[id].reset();
				visible_cards.push_back(&card_sprites[id]);
				if (listener->allow_suit() == any_suit || listener->allow_suit() == card_sprites[id].suit()) {
					card_sprites[id].activate();
				}
				card_position.x += PADDING + width;
			}
		}
	}
}