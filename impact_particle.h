#pragma once
#include "BLIB\particles.h"

class impact_particles : public BLIB::particles {
	const float max_lifetime = 0.75f;
	const float max_height = 2.0f;
	const float radius = 0.8f;
	float delay;

	const BLIB::texture_animation template_animation{ "idle", {
		{{0, 0}, max_lifetime * 0.1667f},
		{{1, 0}, max_lifetime * 0.1667f},
		{{2, 0}, max_lifetime * 0.1667f},
		{{3, 0}, max_lifetime * 0.1667f},
		{{4, 0}, max_lifetime * 0.1667f},
		{{5, 0}, max_lifetime * 0.1667f},
	} };

	impact_particles* clone_impl() const override { return new impact_particles(delay); }

	void update_each(float elapsed_time, particle& particle) override {}

	void emit(float elapsed_time) override {
		delay -= elapsed_time;
		if (delay > 0) return;

		stop();

		particle& p = particle_buffer.emplace_back();
		p.p.uv_size = { 1.0f / 6.0f, 1.0f };

		float angle = 0.5f * PID2 + (PI * 0.75f);
		p.p.position.x = sinf(angle) * radius;
		p.p.position.z = cosf(angle) * radius;
		p.p.position.y = 0.5f * max_height;
		p.scale = float2{2.0f};

		p.animator.add_animation(template_animation);
		p.animator.animate("idle", false);

		p.max_lifetime = max_lifetime;
	}

public:
	impact_particles(float delay) : delay(delay) { load_texture("particles/smack.png"); }
};
