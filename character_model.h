#pragma once
#include "BLIB\entity.h"
#include "BLIB\camera.h"
#include "character_behavior.h"
#include <queue>
#include <typeindex>

class character_model : public BLIB::full::object {
private:
	float3 home;
	bool faces_forward;

	std::queue			<std::unique_ptr<character_behavior>>	state_queue		= {};
	std::unordered_set	<std::type_index>						in_queue		= {};

	template <typename T, typename... Args>
	void enqueue(Args&&... args) {
		state_queue.push(std::make_unique<T>(this, std::forward<Args>(args)...));
		state_queue.back()->add_required();
		in_queue.insert(typeid(T));
	}

	void dequeue();

	void next_state();

public:
	character_model() { assert(false); } // to be nice to set only
	character_model(float3 home, string filename, string alt_texture = "none", bool faces_forward = true);
	character_model(character_model&& o) noexcept : BLIB::full::object(o) {
		home = o.home;
		faces_forward = o.faces_forward;
		in_queue = o.in_queue;
		while (o.state_queue.size() > 0) {
			state_queue.push(std::move(o.state_queue.front()));
			o.state_queue.pop();
			state_queue.back()->parent = this;
		}
	}

	void update(float elapsed_time);

	template <typename T, typename... Args>
	void start_behavior(Args&&... args) {
		enqueue<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	bool behavior_is() {
		return typeid(*(state_queue.front().get())) == typeid(T);
	}

	template <typename T>
	bool behavior_scheduled() {
		return behavior_is<T>() || in_queue.find(typeid(T)) != in_queue.end();
	}

	int get_queue_size() const { return (int)state_queue.size(); }

	float3 get_home() { return home; }
	quaternion get_forward() { if (faces_forward) return quaternion::identity(); else return quaternion::face_to({ 0, 0, 1 }, { 0, 0, -1 }); }
	
	float2 get_screen_pos(BLIB::camera* c);

};