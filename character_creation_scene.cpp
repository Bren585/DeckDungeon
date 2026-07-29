#include "character_creation_scene.h"
#include "mouse_collider.h"
#include "BLIB\audio.h"
#include <fstream>

string class_names[class_count] = {
	"Armorer",
	"Gambler",
	"Mage",
	"Monk",
	"Priest",
	"Shaman",
	"Summoner",
	"Swordsman"
};

string class_model_filenames[class_count] = {
	"Knight",
	"Rogue",
	"Mage",
	"Ranger",
	"Mage",
	"Druid",
	"Rogue",
	"Barbarian"
};

string class_alts[class_count] = {
	"none",
	"none",
	"none",
	"C",
	"A",
	"none",
	"C",
	"A"	
};

std::vector<string> class_descs[class_count]{};

constexpr float classname_scale = 1.0f;
constexpr float classdesc_scale = 0.5f;
constexpr float textbox_scale = 0.75f;
constexpr float button_scale = 0.5f;
constexpr float confirm_scale = 1.0f;

void character_creation_scene::init() {
	//シーン設定
	set_background({0.5f, 0.5f, 0.5f});
	set_camera(&cam);
	get_scene_lights().set_ambient_intensity(2);
	get_scene_lights().set_ambient_color(WHITE);

	//定義
	const float padding			= BLIB::window::size().x / 16.0f;
	const float half_padding	= padding * 0.5f;
	const float quarter_padding = padding * 0.25f;
	const float font_height		= BLIB::text::height();

	//完了バターン
	confirm.make_dummy(WHITE);
	confirm.pos = { BLIB::window::size().x * 0.5f, padding * 2 };
	confirm.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, confirm.get_size() * 0.5f));
	confirm.set_size((float2(BLIB::text::width("Create"), font_height) + float2(half_padding)) * confirm_scale);
	confirm.pivot = C_CC;

	float2 pos = BLIB::window::size() - float2{ padding + half_padding };

	//名前入力
	textbox.set_background(WHITE);
	textbox.resize((float2(BLIB::text::width("IHaveAVeryLongName"), font_height) + float2(half_padding)) * textbox_scale);
	textbox.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, textbox.get_size() * 0.5f));
	textbox.get_collider()->set_off(textbox.get_size() * -0.5f);
	textbox.pivot = C_TR;
	textbox.pos = pos;
	pos.y -= (textbox.get_size().y + half_padding);
	pos.x -= textbox.get_size().x * 0.5f;

	//クラス選択バターン
	float2 button_size = (float2( BLIB::text::width("LongClassName"), font_height ) + float2(quarter_padding)) * button_scale;
	for (int i = 0; i < class_count; i++) {
		auto& button = buttons.emplace_back();
		button.make_dummy({ 0.8f, 0.8f, 0.8f });
		button.set_collider(new BLIB::flat::aligned_rect_collider(nullptr, button.get_size() * 0.5f));
		button.set_size(button_size);
		button.get_collider()->set_off(button.get_size().oy() * -0.5f);
		button.pivot = C_TC;
		button.pos = pos;
		pos.y -= button.get_size().y + quarter_padding;
	}

	//モデルロード
	for (int i = 0; i < class_count; i++) {
		auto&	model		= class_models.emplace_back();
		string	filename	= class_model_filenames[i];
		string	alt_texture	= class_alts[i];

		model.set_model(BLIB::load_fbx(filename, true, BLIB::LH_Y));
		model.add_pos({ 0, -1, 0 });

		//あればテキスチャー切り替え
		if (alt_texture != "none") {
			string texture = filename;
			texture[0] = tolower(texture[0]);
			texture += "_texture_alt_" + alt_texture + ".png";

			for (auto& mat : model.get_model()->get_textures()) {
				if (mat.first == 0) continue;
				mat.second.textures[BLIB::texture_map] = std::make_unique<BLIB::material_texture_file>(texture);
				mat.second.force_construct();
				break;
			}
		}

		//アイドルアニメーション
		add_animations_fbx(model.get_model(), "animations/Rig_Medium_General", 0);

		model.get_model()->animate("Idle_A", 0, true);
		model.set_qtn(quaternion::face_to({ 0, 0, 1 }, { 0, 0, -1 }));
	}

	//クラスの説明を用意
	for (int i = 0; i < class_count; i++) {
		string filename = string("data/desc/", i, ".txt");
		std::ifstream ifs((const char*)filename);

		//後フォーマット出来るため、言葉一個一個読み込む
		char buffer[256];
		while (ifs >> buffer) {
			class_descs[i].push_back(string(buffer, " "));
		}
	}

}

void character_creation_scene::update(float elapsed_time) {
	//入力
	auto* mouse = get_mouse_collider();
	bool click = BLIB::input::trigger(key::LClick);
	if (typing) { name = BLIB::input::get_typing_buffer(); }
	
	//名前入力
	textbox.update(0); //コライダーシンクする
	if (BLIB::collision::check(mouse, &textbox)) {
		if (click) { 
			BLIB::audio::play("click");
			typing = true; 
			BLIB::input::get_typing_buffer() = name; // バッファークリア
		}
	}
	else if (click) { typing = false; }//外にクリックすると、名前入力を終了する

	//クラス選択
	for (int i = 0; i < class_count; i++) {
		buttons[i].update(0);//コライダーシンクする
		if (BLIB::collision::check(mouse, &buttons[i])) { //マウスオーバー
			if (click)					{ selected_class = (character_class)i; buttons[i].tint = { 0.8f, 1.0f, 0.8f }; BLIB::audio::play("click"); }
			if (selected_class != i)	{ buttons[i].tint = { 0.8f, 0.8f, 0.8f }; }
		} 
		//マウスオーバーではない
		else if (selected_class == i)	{ buttons[i].tint = { 0.8f, 1.0f, 0.8f }; } 
		else							{ buttons[i].tint = WHITE; }
	}

	//確定
	confirm.update(0);//コライダーシンクする
	if (name != "") { //名前がなければ確定不可能
		//マウスオーバー
		if (BLIB::collision::check(mouse, &confirm))	{ confirm.tint = { 0.6f, 0.8f, 0.6f }; if (click) { BLIB::manager::stage(main_scene_id, 0, BLIB::transition::fade, 0.5f); BLIB::audio::play("click"); } }
		//マウスオーバーでない
		else											{ confirm.tint = { 0.8f, 1.0f, 0.8f }; }
	}
	else {
		confirm.tint = float3{ 0.8f };
	}

	//選択したクラスのモデルをアップデート
	class_models[selected_class].update(elapsed_time);
}

void character_creation_scene::idle(float elapsed_time) {
	//選択したクラスのモデルをアップデート
	class_models[selected_class].update(elapsed_time);
	confirm.tint = float3{ 0.8f };
}

void character_creation_scene::draw(BLIB::render_settings rs) const {
	//３Dなものはクラスモデルのみ
	//選択したクラスのモデルを描画
	class_models[selected_class].render(rs);
}

void character_creation_scene::draw_transparent() const {
	//2D UI描画

	//定義
	const float padding = BLIB::window::size().x / 16.0f;
	const float half_padding = padding * 0.5f;
	const float quarter_padding = padding * 0.25f;
	const float sixteenth_padding = padding * 0.0625f;
	const float font_height = BLIB::text::height();

	{
		//名前入力
		annotate("Name");
		type("Your name:", textbox.pos + float2{ -textbox.get_size().x, half_padding }, float2{ textbox_scale }, FONT_DEFAULT, WHITE, C_TL);
		textbox.clear();
		textbox.type(name + (((int)timer) % 2 && typing ? "|" : ""), float2{ quarter_padding }, float2{ textbox_scale }, FONT_DEFAULT, BLACK);
		textbox.render();
		//textbox.peek_collider()->render_debug({});
	}

	{
		//クラス選択
		annotate("Buttons");
		for (int i = 0; i < class_count; i++) {
			buttons[i].render();
			//buttons[i].peek_collider()->render_debug({});
			type(class_names[i], buttons[i].pos - float2{ 0, sixteenth_padding }, float2{ button_scale }, FONT_DEFAULT, color(0.3f, 0.3f, 0.3f), C_TC);
		}
	}

	{
		//クラス説明
		annotate("Class Desc");
		float2 pen{ padding, BLIB::window::size().y - padding * 2 };
		float max_width = padding * 5;

		//クラス名
		pen.y -= type(class_names[selected_class], pen, float2{ classname_scale }, FONT_DEFAULT, WHITE, C_TL);
		pen.y -= padding;

		const std::vector<string>& desc = class_descs[selected_class];

		//一個一個言葉を出力して、一列超えたら下へ
		for (auto& word : desc) {
			float w = BLIB::text::width(word) * classdesc_scale; //長さ確認
			if (pen.x + w > max_width) {//超えちゃう！
				pen.x = padding;
				pen.y -= font_height * classdesc_scale + quarter_padding;
			}
			type(word, pen, float2(classdesc_scale), FONT_DEFAULT, WHITE, C_TL);
			pen.x += w;
		}
	}
	//確定
	confirm.render();
	type("Create", confirm.pos + float2(0, sixteenth_padding), float2(confirm_scale), FONT_DEFAULT, BLACK, C_CC);
}

character_data character_creation_scene::get_character_data() {
	return { selected_class, name };
}

