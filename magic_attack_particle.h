#pragma once
#include "BLIB\particles.h"

class magic_attack_particles : public BLIB::particles {
	const float max_lifetime = 0.75f;
	const float max_height = 2.0f;
	const float radius = 0.8f;

	const float max_cooldown = 0.1f;
	float cooldown = 0;

	const float duration = 0.3f;
	float timer = 0;

	const BLIB::texture_animation template_animation{ "idle", {
		{{0, 0}, max_lifetime * 0.2f},
		{{1, 0}, max_lifetime * 0.2f},
		{{2, 0}, max_lifetime * 0.25f},
		{{3, 0}, max_lifetime * 0.25f},
		{{4, 0}, max_lifetime * 0.1f},
	} };

	magic_attack_particles* clone_impl() const override { return new magic_attack_particles(); }

	void update_each(float elapsed_time, particle& particle) override {}

	void emit(float elapsed_time) override {
		timer += elapsed_time;
		cooldown -= elapsed_time;
		if (cooldown > 0) return;
		cooldown += max_cooldown;

		particle& p = particle_buffer.emplace_back();
		p.p.uv_size = { 0.2f, 1.0f };

		float angle = (float)rand() / RAND_MAX * PID2 + (PI * 0.75f);
		p.p.position.x = sinf(angle) * radius;
		p.p.position.z = cosf(angle) * radius;
		p.p.position.y = max_height;
		p.scale = float2{ 1.25f, 3.0f };

		p.animator.add_animation(template_animation);
		p.animator.animate("idle", false);

		p.max_lifetime = max_lifetime;

		if (timer > duration) stop();
	}

public:
	magic_attack_particles() { load_texture("particles/fire_fall.png"); }
};
