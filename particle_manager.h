#pragma once
#include "BLIB\particles.h"
#include "BLIB\object.h"

class particle_manager {
	std::vector<std::unique_ptr<BLIB::full::object>> particles;

public:
	void update(float elapsed_time) {
		for (auto it = particles.begin(); it != particles.end();) {
			(*it)->update(elapsed_time);
			BLIB::particles* p = static_cast<BLIB::particles*>((*it)->get_model());
			if (p->is_active() || p->has_particles()) {
				it++;
			}
			else {
				it = particles.erase(it);
			}
		}
	}

	void render(BLIB::render_settings rs) const { for (auto rit = particles.rbegin(); rit != particles.rend(); rit++) { (*rit)->render(rs); } }

	void add(BLIB::particles* new_particle, float3 at) { 
		particles.emplace_back(std::make_unique<BLIB::full::object>());
		auto new_obj = particles[particles.size() - 1].get(); 
		new_obj->set_model(new_particle);
		new_obj->set_pos(at);
	}
};