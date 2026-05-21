#pragma once
#include "BLIB\particles.h"

class skill_activate_particles : public BLIB::particles {
	const float max_lifetime = 0.25f;
	const float max_height = 2.0f;
	const float radius = 0.8f;
	const int wave_cap = 8;
	int wave_count = 0;

	const float max_cooldown = 0.05f;
	float cooldown = 0;

	const BLIB::texture_animation template_animation{ "idle", {
		{{0, 0}, max_lifetime * 0.25f},
		{{1, 0}, max_lifetime * 0.25f},
		{{2, 0}, max_lifetime * 0.25f},
		{{3, 0}, max_lifetime * 0.25f}
	} };

	skill_activate_particles* clone_impl() const override { return new skill_activate_particles(); }

	void update_each(float elapsed_time, particle& particle) override {
		
	}

	void emit(float elapsed_time) override {
		cooldown -= elapsed_time;
		if (cooldown > 0) return;
		cooldown = max_cooldown;
		wave_count++;

		particle& p = particle_buffer.emplace_back();
		p.animator.add_animation(template_animation);
		p.animator.animate("idle", false);
		p.max_lifetime = max_lifetime;
		p.p.uv_size = { 0.25f, 1 };

		const float t = (float)wave_count / (float)wave_cap;
		p.p.position.y = max_height * t;
		p.p.position.x = cosf(t * PI2) * radius;
		p.p.position.z = sinf(t * PI2) * radius;
		p.scale = float2{ 0.75f }; 
	
		if (wave_count >= wave_cap) stop();
	}

public:
	skill_activate_particles() { load_texture("particles/magic_trail.png"); }
};
