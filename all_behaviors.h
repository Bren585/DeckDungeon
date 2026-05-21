#pragma once
#include "character_behavior.h"
#include "dungeon_scene.h"
#include "all_particles.h"

constexpr float social_distancing = 2.5f;
constexpr float jump_height = 12.0f;
constexpr float jump_height_determinant = -jump_height / 4;

inline void make_particle(BLIB::particles* new_particle, float3 pos) { static_cast<dungeon_scene*>(BLIB::manager::get_scene(BLIB::manager::find_first_of_type<dungeon_scene>()))->get_pm().add(new_particle, pos); }

namespace cb {
	/* Idle **********************************************************************************************************************/

	class idle : public character_idle_behavior {
	public:
		idle(character_model* parent) : character_idle_behavior(parent, "Idle_A") {}
	};

	/* Relax *********************************************************************************************************************/

	class relax : public character_idle_behavior {
	public:
		relax(character_model* parent, string relax_type) : character_idle_behavior(parent, relax_type + "Idle", 2) {}
	};

	class relax_end : public character_animation_behavior {
	public:
		relax_end(character_model* parent, string relax_type) : character_animation_behavior(parent, relax_type + "StandUp") {}
	};

	constexpr int relax_type_count = 2;
	const string relax_types[relax_type_count] = { "Sit_Floor_", "Lie_" };
	class relax_start : public character_animation_behavior {
		int relax_type_index;
		void add_required() override {
			parent->start_behavior<relax>(relax_types[relax_type_index]);
			parent->start_behavior<relax_end>(relax_types[relax_type_index]);
		}
	public:
		relax_start(character_model* parent, int relax_type_index = rand() % relax_type_count) :
			relax_type_index(relax_type_index),
			character_animation_behavior(parent, relax_types[relax_type_index] + "Down") 
		{}
	};

	/* Movement *****************************************************************************************************************/

	class land : public character_animation_behavior {
	public:
		land(character_model* parent) : character_animation_behavior(parent, "Jump_Land") {}
	};

	class go_to : public character_behavior {
	private:
		float3 begin;
		const float3 end;
		const float duration;
		float timer;

		void enter() override {
			parent->get_model()->animate("Jump_Start", 0.2f);
			begin = parent->get_pos();
		}

		void main(float elapsed_time) override {
			timer += elapsed_time;
			float t = timer / duration;
			if (t > 1) { t = 1; }
			parent->set_pos(lerp(begin, end, t));
			parent->add_pos(float3{ 0, (jump_height_determinant * t * (t - 1)), 0 });
		}

		bool condition() override { return timer > duration; }

		void exit() override { parent->set_pos(end); }

		void add_required() override { parent->start_behavior<land>(); }

	public:
		go_to(character_model* parent, float3 destination, float duration) :
			character_behavior(parent),
			end(destination),
			duration(duration),
			timer(0)
		{
		}
	};

	class go_home : public go_to {
	public:
		go_home(character_model* parent, float duration) : go_to(parent, parent->get_home(), duration) {}
	};

	class face_to : public character_behavior {
		quaternion begin;
		const quaternion end;
		const float duration;
		float timer;

		void enter() override {
			begin = parent->get_qtn();
		}

		void main(float elapsed_time) override {
			timer += elapsed_time;
			float t = timer / duration;
			if (t > 1) { t = 1; }
			parent->set_qtn(slerp(begin, end, t));
		}

		bool condition() override { return timer > duration; }

		void exit() override { parent->set_qtn(end); }

	public:
		face_to(character_model* parent, float3 target, float duration) :
			character_behavior(parent),
			end(quaternion::face_to({ 0, 0, 1 }, (target - parent->get_pos()).norm())),
			duration(duration),
			timer(0)
		{
		}
	};

	class face_forward : public character_behavior {
		quaternion begin;
		const quaternion end;
		const float duration;
		float timer;

		void enter() override {
			begin = parent->get_qtn();
		}

		void main(float elapsed_time) override {
			timer += elapsed_time;
			float t = timer / duration;
			if (t > 1) { t = 1; }
			parent->set_qtn(slerp(begin, end, t));
		}

		bool condition() override { return timer > duration; }

		void exit() override { parent->set_qtn(end); }

	public:
		face_forward(character_model* parent, float duration) :
			character_behavior(parent),
			end(parent->get_forward()),
			duration(duration),
			timer(0)
		{
		}
	};

	class summon : public character_behavior {
		const float3 begin;
		const float3 end;
		float duration{0};
		float timer;

		void enter() override {
			parent->get_model()->animate("Ranged_Magic_Summon", 0, false, 2.0f);
			duration = parent->get_model()->get_animation("Ranged_Magic_Summon").get_play_duration();
			make_particle(new summon_particles(duration * 0.75f), parent->get_pos() * float3 { 1, 0, 1 });
		}

		void main(float elapsed_time) override {
			timer += elapsed_time;
			float t = timer / duration;
			if (t > 1) { t = 1; }
			parent->set_pos(lerp(begin, end, t));
		}

		bool condition() override { return timer > duration; }

		void exit() override { parent->set_pos(end); }

		void add_required() override { parent->start_behavior<land>(); }

	public:
		summon(character_model* parent) :
			character_behavior(parent),
			begin(parent->get_pos() - float3{ 0, 2, 0 }),
			end(parent->get_pos()),
			timer(0)
		{
			parent->set_pos(begin);
		}
	};

	/* Melee ********************************************************************************************************************/

	class punch : public character_animation_behavior {
		character_model* target;
		void enter() override {
			character_animation_behavior::enter();
			make_particle(new impact_particles(parent->get_model()->get_animation("Melee_Unarmed_Attack_Punch_A").get_duration() * 0.5f), target->get_pos());
		}
	public:
		punch(character_model* parent, character_model* target) : character_animation_behavior(parent, "Melee_Unarmed_Attack_Punch_A"), target(target) {}
	};

	class melee_attack : public character_behavior_set {
		character_model* target;
		void add_required() override {
			float3 parent_pos = parent->get_pos();
			float3 target_pos = target->get_pos();
			float3 to_target = target_pos - parent_pos;
			float dist = to_target.mag();
			to_target *= (dist - social_distancing) / dist;
			float3 land_pos = parent_pos + to_target;

			parent->start_behavior<face_to>(target_pos, 0.25f);
			parent->start_behavior<go_to>(land_pos, 0.5f);
			parent->start_behavior<punch>(target);
			parent->start_behavior<go_home>(0.5f);
			parent->start_behavior<face_forward>(0.25f);
		}
	public:
		melee_attack(character_model* parent, character_model* target) : 
			character_behavior_set(parent),
			target(target)
		{	
		}
	};

	class block : public character_idle_behavior {
	public:
		block(character_model* parent) : character_idle_behavior(parent, "Melee_Unarmed_Idle") {}
	};

	/* Magic ********************************************************************************************************************/

	class casting : public character_behavior {
		character_model* target;
		const float duration;
		float timer = 0;

		void enter() override {
			parent->get_model()->animate("Ranged_Magic_Spellcasting", true);
			make_particle(new magic_trail_particles(target->get_pos() - parent->get_pos(), duration), parent->get_pos());
		}

		void main(float elapsed_time) override { timer += elapsed_time; }

		bool condition() override { return timer > duration; }

		void exit() override { make_particle(new fireball_particles(), target->get_pos()); }

	public:
		casting(character_model* parent, character_model* target, float duration) : character_behavior(parent), target(target), duration(duration) {}
	};

	class shoot : public character_animation_behavior {
	public:
		shoot(character_model* parent) : character_animation_behavior(parent, "Ranged_Magic_Shoot") {}
	};

	class magic_attack : public character_behavior_set {
		character_model* target;
		void add_required() override {
			parent->start_behavior<face_to>(target->get_pos(), 0.25f);
			parent->start_behavior<casting>(target, 1.0f);
			parent->start_behavior<shoot>();
			parent->start_behavior<face_forward>(0.25f);
		}
	public:
		magic_attack(character_model* parent, character_model* target) : 
			character_behavior_set(parent),
			target(target)
		{
		}
	};

	class magic_skill : public character_animation_behavior {
		void enter() override {
			character_animation_behavior::enter();
			make_particle(new skill_activate_particles, parent->get_pos());
		}
	public:
		magic_skill(character_model* parent) : character_animation_behavior(parent, "Ranged_Magic_Raise") {}
	};

	/* Death ********************************************************************************************************************/

	class stay_dead : public character_idle_behavior {
	public:
		stay_dead(character_model* parent) : character_idle_behavior(parent, "Death_A_Pose") {}
	};

	class die : public character_animation_behavior {
		void add_required() override { parent->start_behavior<stay_dead>(); }
	public:
		die(character_model* parent) : character_animation_behavior(parent, "Death_A") {}
	};

	class disappear : public character_behavior {
		const float3 begin;
		const float3 end;
		const float duration;
		float timer;

		void main(float elapsed_time) override {
			timer += elapsed_time;
			float t = timer / duration;
			if (t > 1) { t = 1; }
			parent->set_pos(lerp(begin, end, t));
		}

		bool condition() override { return timer > duration; }

		void exit() override { parent->set_pos(end); }

		void add_required() override { parent->start_behavior<stay_dead>(); }

	public:
		disappear(character_model* parent) :
			character_behavior(parent),
			begin(parent->get_pos()),
			end(parent->get_pos() - float3{ 0, 1, 0 }),
			duration(1.0f),
			timer(0)
		{
		}
	};

	/* Others *******************************************************************************************************************/

	class celebrate : public character_idle_behavior {
	public:
		celebrate(character_model* parent) : character_idle_behavior(parent, "Cheering") {}
	};
}