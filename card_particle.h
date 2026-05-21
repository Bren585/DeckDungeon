#pragma once
#include "BLIB\particles.h"
#include "cards.h"

class card_particle : public BLIB::particles {
	const float float_speed = 1.5f;
	const float rotation_speed = 3.0f;

	card c;
	card_particle* clone_impl() const override { return new card_particle(c); }

	void update_each(float elapsed_time, particle& particle) override {
		particle.p.position.y += float_speed * elapsed_time;
		float t = particle.lifetime / particle.max_lifetime;
		float shrink = (1 - powf(2, 40 * (t - 1)));
		if (shrink < 0) { shrink = 0; }
		particle.scale.x = fabsf(cosf(particle.lifetime * rotation_speed)) * shrink;
		particle.scale.y = shrink;
	}

	void emit(float elapsed_time) override { 
		stop();
		particle& particle = particle_buffer.emplace_back();
		particle.p.size = { 29.0f / 40.0f * 2.0f, 2 };
		particle.max_lifetime = 2.0f;
	}

public:
	card_particle(card c) : c(c) { load_texture(c.sprite_name()); }
};

