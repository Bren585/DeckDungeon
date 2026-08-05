#pragma once
#include "BLIB\particles.h"
#include "BLIB\canvas.h"

class number_particle : public BLIB::particles { 
	const float slide_dist = 2.5f;
	const float bounce_top = 1.5f;
	const float bounce_strength = 0.6f;
	const float gravity = -9.8f;
	int amount;
	color tint;
	float2 size;

	number_particle* clone_impl() const override { return new number_particle(amount, tint); }

	void update_each(float elapsed_time, particle& particle) override {
		float t = particle.lifetime / particle.max_lifetime;
		// There are three controls here:
		// Slide to the right on X, slowing down.
		// Bounce on Y, bounces getting smaller
		// Dissappear on scale, right as time is running out.
		particle.velocity.y += gravity * elapsed_time;
		particle.p.position.y += particle.velocity.y * elapsed_time;
		if (particle.p.position.y < 0) {
			particle.p.position.y *= -1;
			particle.velocity.y *= -bounce_strength;
		}

		particle.p.position.x = slide_dist * (1 - powf(1 - t, 4));
		particle.scale = float2(1 - powf(2, 10 * (t - 1)));
	}

	void emit(float elapsed_time) override {
		stop();
		particle& particle = particle_buffer.emplace_back();
		particle.p.position.y = bounce_top;
		particle.p.size = size * float2(0.03125f);
		//particle.scale = float2(0.0125f);
		particle.max_lifetime = 2.0f;
	}

public:
	number_particle(int a, color c) : amount(a), tint(c) {
		string text(a);
		size = { BLIB::text::width(text), BLIB::text::height() };
		BLIB::canvas snapshot(size);
		//snapshot.set_background({ 0, 0, 0, 0 });
		//snapshot.clear();
		snapshot.type(text, float2(0), float2(1), FONT_DEFAULT, c);
		copy_texture(snapshot.peek_sprite()->peek_SRV());
	}
};

