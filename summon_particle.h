#pragma once
#include "BLIB\particles.h"

class summon_particles : public BLIB::particles {
	const float max_lifetime = 0.75f;
	const float gravity = 6.0f;
	const float velocity_up = 2.0f;
	const float velocity_side = 1.5f;

	const float max_cooldown = 0.15f;
	float cooldown = 0;

	const float duration;
	float timer = 0;

	const BLIB::texture_animation template_animation{ "idle", {
		{{0, 0}, max_lifetime * 0.1667f},
		{{1, 0}, max_lifetime * 0.1667f},
		{{2, 0}, max_lifetime * 0.1667f},
		{{3, 0}, max_lifetime * 0.1667f},
		{{4, 0}, max_lifetime * 0.1667f},
		{{5, 0}, max_lifetime * 0.1667f},
	} };

	summon_particles* clone_impl() const override { return new summon_particles(duration); }

	void update_each(float elapsed_time, particle& particle) override {
		particle.velocity.y -= gravity * elapsed_time;
		particle.p.position += particle.velocity * elapsed_time;
	}

	void emit(float elapsed_time) override {
		timer += elapsed_time;
		cooldown -= elapsed_time;
		if (cooldown > 0) return;
		cooldown = max_cooldown;

		particle& p = particle_buffer.emplace_back();
		p.animator.add_animation(template_animation);
		p.animator.animate("idle", true);
		p.max_lifetime = max_lifetime;
		p.p.uv_size = { 1.0f / 6.0f, 1 };

		const float angle = (float)rand() / RAND_MAX * PI2;
		p.velocity.y = velocity_up;
		p.velocity.x = cosf(angle) * velocity_side;
		p.velocity.z = sinf(angle) * velocity_side;

		if (timer > duration) stop();
	}

public:
	summon_particles(float duration) : duration(duration) { load_texture("particles/smoke.png"); }
};
