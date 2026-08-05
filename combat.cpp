#include "combat.h"
#include "log.h"

#include "shaman.h"

#include "choose_action.h"
#include "choose_target.h"
#include "choose_skill.h"

enum combat_checkpoint {
	start_round,
	start_turn,
	start_action,
	start_reaction,
	killed_self,
	end_action,
	end_turn,
	end_round,
	end_floor,
};

void combat::init() {
	for (auto& player : get_players()) {
		if (player->has_flag(spiritual)) {
			shaman_player = static_cast<shaman*>(player);
			break;
		}
	}
}

void combat::update(float elapsed_time) {
	auto& players = get_players();
	auto& enemies = get_enemies();
	auto& attackers = player_phase ? players : enemies;
	auto& defenders = player_phase ? enemies : players;

	BEGIN_COROUTINE;

	SET_CHECKPOINT(start_round);

	// ラウンドスタート、フェース切り替え
	player_phase = !player_phase;

	event_log::record("New round begins!");
	event_log::record(player_phase ? "Player Phase" : "Enemy Phase");

	WAIT(0.25f);

	COIT = 0;
	SET_CHECKPOINT(start_turn);
	// ターンスタート

	attacker = attackers[COIT];

	if (attacker->dead()) { // 死んでればスキップ
		GO_TO_CHECKPOINT(end_turn);
	}

	//　ターン用意
	event_log::record(string(attacker->get_name(), "'s turn"));
	attacker_discard.empty();
	target_discard.empty();

	SET_CHECKPOINT(start_action);
	//　アクション選択

	get_scene()->spotlight(attacker->get_id());

	if (attacker->has_flag(cannot_attack)) { // 普通攻撃できない 
		if (attacker->get_hand().size() == 0) { // スキルのため、カードもなければスキップ
			event_log::record(string(attacker->get_name(), " cannot act!"));
			GO_TO_CHECKPOINT(end_turn);
		}
		choice = use_skill; //　普通攻撃できないのでスキルしかない
	} 
	else if (attacker->has_flag(mindless)) { //　ランダム選択
		choice = random_action;
	}
	else { // プレイヤーがアクションを選択
		YIELD_SUBTASK(choose_action, this, attacker->get_id(), action_any);
	}

	// 選択結果
	{
		if (choice == use_skill) { // スキル
			// スキル選択
			YIELD_SUBTASK(choose_skill, this, attacker->get_skills());
			if (choice == cancel_skill) { GO_TO_CHECKPOINT(start_action); } // キャンセル、戻る
			attacker_skill = attacker->get_skills()[choice];

			//　どんなカード出せばいいか
			if (attacker_skill.cost != no_suit) {
				action_type cost;
				if		(attacker_skill.cost == spade)		cost = action_pay_spade;
				else if (attacker_skill.cost == heart)		cost = action_pay_heart;
				else if (attacker_skill.cost == club)		cost = action_pay_club;
				else if (attacker_skill.cost == diamond)	cost = action_pay_diamond;
				else										cost = action_pay_any;
				//　プレイヤー選択待ち
				YIELD_SUBTASK(choose_action, this, attacker->get_id(), cost);
				if (choice == cancel_skill) { GO_TO_CHECKPOINT(start_action); } // キャンセル、戻る
			} 
			// アクション決まり
			attacker_action = attacker_skill.effect(attacker->get_hand().play(choice), attacker);
			choice = use_skill; //選択リセット
		}

		if (choice == top_deck) { // 山引く
			attacker_action = attacker->basic_attack(attacker->get_deck().play());
		}
		else if (choice == random_action) { // ランダム
			attacker_action = attacker->basic_attack(rand_card());
		}
		else if (choice >= 0) { //　手札から出す
			attacker_action = attacker->basic_attack(attacker->get_hand().play(choice));
		}
	}

	//　パーティクル生み出す
	get_scene()->get_pm().add(new card_particle(attacker_action.cost), get_scene()->get_character_model(attacker->get_id()).get_pos() + card_spawn_offset);
	get_scene()->clear_spotlight();

	//　ターゲット選択
	{
		// 可能なターゲットを集まる
		targets.clear();
		if (attacker_action.flags & (target_ally | target_all_allies)) { // 仲間
			for (auto& ally : attackers) {
				if (attacker_action.flags & target_dead_allies) { // 死亡者をフィルター
					if (ally->alive()) continue;
				}
				else {
					if (ally->dead()) continue;
				}

				if (attacker_action.flags & target_thrall_only) { // 召喚士のモンスターをフィルター
					if (!(ally->has_flag(is_thrall))) continue;
				}
				
				if (*ally != *attacker || attacker_action.flags & can_target_self) { // 自分でもいい？
					targets.push_back(ally); //　ターゲット可能
				}
			}
		}
		else if (attacker_action.flags & (target_enemy | target_all_enemies)) { // 敵
			for (auto& defender : defenders) { 
				if (defender->dead()) continue;
				if (defender->has_flag(cannot_attack)) continue;
				targets.push_back(defender); 
			}
		}
		else if (attacker_action.flags & can_target_self) { // 自分
			targets.push_back(attacker);
		}
		else {
			assert(false); // ターゲット不可能って有り得ない
		}

		// 可能なターゲットいない
		if (targets.size() == 0) {
			// カードを戻す
			if (!attacker->has_flag(mindless)) {
				if (choice == top_deck) { pile({ attacker_action.cost }).into(attacker_discard); } 
				else { pile({ attacker_action.cost }).into(attacker->get_hand()); }
			}
			event_log::record("Card could not be played.");
			GO_TO_CHECKPOINT(start_action); // やり直し
		}

		// 可能なターゲットから選択
		if (!(attacker_action.flags & multi_target)) { // 一人まで

			if (targets.size() == 1) choice = 0; // 一人しかないので、自動に一人を選択
			else if (attacker->has_flag(mindless)) { // 選択がカードにより
				choice = -1;
				{
					int target_evasion = 1000;
					for (int i = 0; i < targets.size(); i++) {
						//　皆カード引いて、低い方がターゲットされる
						int evasion;
						if (targets[i]->has_flag(mindless)) { // ランダム
							card c = rand_card();
							evasion = targets[i]->get_luck_bonus() + c.face() + targets[i]->get_buffs().luck;
							get_scene()->get_pm().add(new card_particle(c), get_scene()->get_character_model(targets[i]->get_id()).get_pos() + card_spawn_offset);
						}
						else { //　自分の山から
							pile temp;
							targets[i]->get_deck().draw().into(temp);
							evasion = targets[i]->get_luck_bonus() + card(temp).face() + targets[i]->get_buffs().luck;
							get_scene()->get_pm().add(new card_particle(temp), get_scene()->get_character_model(targets[i]->get_id()).get_pos() + card_spawn_offset);
							temp.into(targets[i]->get_deck());
							targets[i]->get_deck().shuffle();
						}

						if (evasion < target_evasion) {
							choice = i;
							target_evasion = evasion;
						}
					}
				}
				assert(choice != -1);
				WAIT(0.5f);
			}
			else {
				//　プレイヤー選択
				YIELD_SUBTASK(choose_target, this, targets);
			}

			// アクション発表
			targets = { targets[choice] };
			event_log::record(string(
				attacker->get_name(),
				" ",
				attacker_action.name,
				" ",
				*targets[0] == *attacker ? string("self") : targets[0]->get_name()
			));
		}
		else { //　全員をターゲット
			// アクション発表
			event_log::record(string(
				attacker->get_name(),
				" ",
				attacker_action.name,
				" ",
				attacker_action.flags & target_all_allies ? "all allies" : "all enemies"
			));
		}
	}

	// バッフ
	{
		if (attacker_action.flags & is_buff) { // バッフあり？
			// スキルアニメーション
			get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::magic_skill>();
			WAIT(0.1f);
			YIELD_WHILE(!get_scene()->get_character_model(attacker->get_id()).behavior_is<cb::idle>());

			{
				//	デバッフチェック
				stats buff = attacker_action.buff;
				bool is_debuff = false;
				for (int i = 0; i < 4; i++) {
					int& stat = buff[i];
					if (stat == 0)	continue;
					if (stat > 0) { is_debuff = false;  break; }
					else		  { is_debuff = true;	break; }
				}

				//　このアクション、ラックにバッフされていい？
				if ((attacker_action.flags & cannot_be_buffed) == 0) {
					int luck_bonus = attacker->get_buffs().luck * is_debuff ? -1 : 1;
					for (int i = 0; i < 4; i++) { if (buff[i] != 0) buff[i] += luck_bonus; }
				}

				//　ターゲットにバッフを与え
				for (auto target : targets) {
					if		(attacker_action.flags & is_turn_buff ) { target->buff_for_turn (buff); }
					else if (attacker_action.flags & is_round_buff) { target->buff_for_round(buff); }
					else if (attacker_action.flags & is_floor_buff) { target->buff_for_floor(buff); }

					//　復活？
					if (attacker_action.flags & target_dead_allies && target->alive()) {
						get_scene()->get_character_model(target->get_id()).start_behavior<cb::relax_end>("Lie_");
					}
				}

				//　パーティクル生み出す
				for (auto target : targets) {
					if (attacker_action.flags & magic_attack) {
						get_scene()->get_pm().add(
							new magic_attack_particles(),
							get_scene()->get_character_model(target->get_id()).get_pos()
						);
					}
					else {
						if (is_debuff) {
							get_scene()->get_pm().add(
								new debuff_particles(),
								get_scene()->get_character_model(target->get_id()).get_pos()
							);
						}
						else {
							get_scene()->get_pm().add(
								new buff_particles(), 
								get_scene()->get_character_model(target->get_id()).get_pos()
							);
						}
					}
				}
			}			
		}
	}

	// 攻撃
	if (attacker_action.flags & is_combat) {
		get_scene()->spotlight(targets[0]->get_id(), RED);

		// リアクション
		SET_CHECKPOINT(start_reaction);

		if (targets[0]->has_flag(mindless)) { // ランダム
			choice = random_action;
		}
		else { // プレイヤー選択
			YIELD_SUBTASK(choose_action, this, targets[0]->get_id(), action_reaction);
		}

		if (choice == top_deck) { // 山引く
			defender_action = targets[0]->reaction(targets[0]->get_deck().play());
		}
		else if (choice == random_action) { // ランダム
			defender_action = targets[0]->reaction(rand_card());
		}
		else if (choice >= 0) { // 手札から出す
			defender_action = targets[0]->reaction(targets[0]->get_hand().play(choice));
		}

		event_log::record(string(targets[0]->get_name(), " ", defender_action.name));

		if (choice != random_action) {
			// カード片付け
			pile({ defender_action.cost }).into(target_discard);
		}

		//　カードパーティクル
		get_scene()->get_pm().add(new card_particle(defender_action.cost), get_scene()->get_character_model(targets[0]->get_id()).get_pos() + card_spawn_offset);
		get_scene()->clear_spotlight();
		
		// バッフ？
		if (defender_action.flags & is_turn_buff && defender_action.flags & can_target_self) {
			targets[0]->buff_for_turn(defender_action.buff);
			get_scene()->get_pm().add(
				new buff_particles(),
				get_scene()->get_character_model(targets[0]->get_id()).get_pos()
			);
			get_scene()->get_character_model(targets[0]->get_id()).start_behavior<cb::magic_skill>();
			WAIT(0.1f);
			YIELD_WHILE(!get_scene()->get_character_model(targets[0]->get_id()).behavior_is<cb::idle>());
			if (defender_action.flags & act_again) { // ダイアモンド
				GO_TO_CHECKPOINT(start_reaction);
			}
		}

		// カード片付け
		if (!targets[0]->has_flag(mindless)) {
			target_discard.into(targets[0]->get_deck());
			targets[0]->get_deck().shuffle();
		}

		/* 攻撃結果 */ {
			// 攻撃力
			int attack = attacker->get_buffed_stats().attack;
			if (attacker_action.flags & heal_bonus) attacker->heal(attacker_action.cost.modifier() + attacker->get_buffs().luck);
			if (attacker_action.flags & attack_bonus) attack += attacker_action.cost.modifier() + attacker->get_buffs().luck;

			if (attacker->dead()) { // 自殺しちゃった
				event_log::record(string(attacker->get_name(), " tripped and died!"));
				if (shaman_player) { shaman_player->gain_souls(1); }
				attacker->on_death();
				targets[0]->end_turn();

				GO_TO_CHECKPOINT(killed_self); //　攻撃終わらず
			}

			//　防衛力
			int defense = targets[0]->get_buffed_stats().defense;
			if (defender_action.flags & heal_bonus) targets[0]->heal(defender_action.cost.modifier() + targets[0]->get_buffs().luck);
			if (defender_action.flags & defense_bonus) { 
				defense += defender_action.cost.modifier() + targets[0]->get_buffs().luck; 
				get_scene()->get_character_model(targets[0]->get_id()).start_behavior<cb::block>();
			}

			// 結果
			int damage = attack - defense;
			if (damage < 0) damage = 0;
			event_log::record(string("Dealt ", TEXT_COLOR(RED), damage, TEXT_COLOR(WHITE), " damage in combat"));
			targets[0]->damage(damage);
			targets[0]->end_turn();
		}

		// パーティクル生み出す
		if (attacker_action.flags & magic_attack) {
			get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::magic_attack>(&get_scene()->get_character_model(targets[0]->get_id()));
		}
		else {
			get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::melee_attack>(&get_scene()->get_character_model(targets[0]->get_id()));
		}

		WAIT(0.25f);
		//アニメーション終わりまで待つ
		YIELD_WHILE(!get_scene()->get_character_model(attacker->get_id()).behavior_is<cb::idle>());
		get_scene()->get_character_model(targets[0]->get_id()).start_behavior<cb::idle>();
	}

	// 死亡者チェック、レベルアップ与え
	if (attacker_action.flags & can_kill) {
		for (auto defender_it = defenders.begin(); defender_it != defenders.end(); ) {
			if ((*defender_it)->dead()) {
				//すでに死んだものをスキップ
				if (get_scene()->get_character_model((*defender_it)->get_id()).behavior_scheduled<cb::stay_dead>()) { defender_it++; continue; }
				
				//死んでるアニメーション
				get_scene()->get_character_model((*defender_it)->get_id()).start_behavior<cb::die>();
				event_log::record(string((*defender_it)->get_name(), " died!"));

				//　レベルアップ
				attacker->award_exp(1);

				//　シャーマン
				if (shaman_player) { shaman_player->gain_souls(1); }

				//　死んだ敵を消す
				if (player_phase) {
					get_scene()->get_character_model((*defender_it)->get_id()).start_behavior<cb::disappear>();
					delete (*defender_it);
					defender_it = defenders.erase(defender_it);
					continue;
				}
			}
			defender_it++;
		}
	}

	GO_TO_CHECKPOINT(end_action);

	// 攻撃中自殺
	SET_CHECKPOINT(killed_self);

	//死んでるアニメーション
	get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::die>();
	YIELD_WHILE(get_scene()->get_character_model(attacker->get_id()).behavior_scheduled<cb::die>());

	if (!player_phase) { //　死んだ敵を消す
		get_scene()->get_character_model(attacker->get_id()).start_behavior<cb::disappear>();
		delete attacker;
		enemies.erase(enemies.begin() + COIT--);
		GO_TO_CHECKPOINT(end_turn);
	}

	// アクション終了
	SET_CHECKPOINT(end_action);
	// カード片付け
	if (!attacker->has_flag(mindless)) {
		pile({ attacker_action.cost }).into(attacker_discard);
	}
	//　ダイアモンド
	if (attacker_action.flags & act_again && enemies.size() > 0 && attacker->alive()) { GO_TO_CHECKPOINT(start_action); }
	//　ターン終了させる
	attacker->end_turn();
	// カード片付け
	if (!attacker->has_flag(mindless)) {
		attacker_discard.into(attacker->get_deck());
		attacker->get_deck().shuffle();
	}

	SET_CHECKPOINT(end_turn);
	// 手札少なければ、山から引く
	if (player_phase && attacker->alive()) {
		if (attacker->get_hand().size() < 3 && !attacker->has_flag(is_thrall)) {
			attacker->get_deck().draw(1).into(attacker->get_hand());
		}
	}

	// クリアチェック
	{
		bool all_dead = true;
		for (auto& defender : defenders) { if (defender->alive()) { all_dead = false; break; } }
		if (all_dead) { GO_TO_CHECKPOINT(end_floor); }
	}

	WAIT(0.5f);

	COIT++;
	// 全員繰り返し
	if (COIT < attackers.size()) { GO_TO_CHECKPOINT(start_turn); }

	// 敵の番
	if (player_phase) { GO_TO_CHECKPOINT(start_round); }

	// ラウンド終了
	SET_CHECKPOINT(end_round);

	for (auto& player : players) { player->end_round(); }
	for (auto& enemy : enemies) { enemy->end_round(); }

	GO_TO_CHECKPOINT(start_round);

	// フロア終了
	SET_CHECKPOINT(end_floor);

	for (auto& player : players) { player->end_floor(); }

	finish();
	END_COROUTINE;
}