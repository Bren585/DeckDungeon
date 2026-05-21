#pragma once
#include "BLIB\particles.h"

class debuff_particles : public BLIB::particles {
	const float true_max_lifetime = 1.0f;
	const float min_height = 0.25f;
	const float max_height = 2.0f;
	const float radius = 0.8f;
	const float rotation_speed = PI2 * 2.0f;
	const int particle_count = 4;
	const int wave_cap = 2;
	int wave_count = 0;

	const float max_cooldown = 0.5f;
	float cooldown = 0;

	const BLIB::texture_animation template_animation{ "idle", {
		{{0, 0}, 0.0625f},
		{{1, 0}, 0.0625f},
		{{2, 0}, 0.0625f},
		{{3, 0}, 0.0625f}
	} };

	debuff_particles* clone_impl() const override { return new debuff_particles(); }

	void update_each(float elapsed_time, particle& particle) override {
		const float offset = particle.max_lifetime - true_max_lifetime;
		const float local_lifetime = particle.lifetime - offset;
		const float t = (local_lifetime) / (particle.max_lifetime - offset);
		particle.p.position.y = min_height + max_height * ((t < 0.5f) ? 1 - (8 * t * t * t * t) : powf(-2 * t + 2, 4) * 0.5f);
		particle.p.position.x = -cosf(offset + local_lifetime * rotation_speed) * radius;
		particle.p.position.z = sinf(offset + local_lifetime * rotation_speed) * radius;
		particle.scale = float2{ (1 - powf(2, 40 * (t - 1))) } *1.0f;
	}

	void emit(float elapsed_time) override {
		cooldown -= elapsed_time;
		if (cooldown > 0) return;
		cooldown = max_cooldown;
		wave_count++;

		float offset = PI2 / particle_count;
		for (int i = 0; i < particle_count; i++) {
			particle& p = particle_buffer.emplace_back();
			p.animator.add_animation(template_animation);
			p.animator.animate("idle", true);
			p.lifetime = offset * i;
			p.max_lifetime = true_max_lifetime + p.lifetime;
			p.p.uv_size = { 0.25f, 1 };
		}

		if (wave_count >= wave_cap) stop();
	}

public:
	debuff_particles() { load_texture("particles/purple_star.png"); }
};
