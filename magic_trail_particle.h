#pragma once
#include "BLIB\particles.h"

class magic_trail_particles : public BLIB::particles {
	const float max_lifetime = 0.75f;
	const float gravity = 4.0f;
	const float jiggle_min = -8.0f;
	const float jiggle_mag = 16.0f;

	const float max_cooldown = 0.05f;
	float cooldown = 0;

	const float3 travel_vector;
	const float duration;
	float timer = 0;

	const BLIB::texture_animation template_animation{ "idle", {
		{{0, 0}, max_lifetime * 0.15f},
		{{1, 0}, max_lifetime * 0.2f},
		{{2, 0}, max_lifetime * 0.3f},
		{{3, 0}, max_lifetime * 0.35f}
	} };

	magic_trail_particles* clone_impl() const override { return new magic_trail_particles(travel_vector, duration); }

	void update_each(float elapsed_time, particle& particle) override {
		float2 jiggle = { jiggle_min + ((float)rand() / RAND_MAX) * jiggle_mag, jiggle_min + ((float)rand() / RAND_MAX) * jiggle_mag };
		particle.velocity.x += jiggle.x * elapsed_time;
		particle.velocity.z += jiggle.y * elapsed_time;
		particle.velocity.y -= gravity * elapsed_time;

		particle.p.position += particle.velocity * elapsed_time;
	}

	void emit(float elapsed_time) override {
		timer += elapsed_time;
		cooldown -= elapsed_time;
		if (cooldown > 0) return;
		cooldown += max_cooldown;

		const float t = timer / duration;

		particle& p = particle_buffer.emplace_back();
		p.animator.add_animation(template_animation);
		p.animator.animate("idle", false);
		p.max_lifetime = max_lifetime;
		p.p.uv_size = { 0.25f, 1 };
		p.p.position = travel_vector * t;
		p.p.position.y = 1.3f;
		p.scale = float2{ 0.75f };

		if (timer > duration) stop();
	}

public:
	magic_trail_particles(float3 vec, float duration) : travel_vector(vec), duration(duration) { load_texture("particles/magic_trail.png"); }
};
