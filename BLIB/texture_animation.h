#pragma once
#include "math.h"
#include "cereal.h"
#include <vector>
#include <map>
#include <queue>

/*
	A texture animator is used to handle animations on a 2D texture.
	It has built in integration with flat::object and particles.

	It's usage is very simalar to the animator used in models. Animations
	can be queued by name with the animate(...) function, skipped with the
	next() function, or completely stopped with the stop_all() function.

	A texture animation is the data required to animate a texture.
	In short, it's a collection of keyframes, made up of the uv_index of each
	frame of an animation in a tilemap and the duration that frame should last.
*/

namespace BLIB {

	struct texture_frame {
		float2	uv_index;
		float	duration;

		SERIALIZE(uv_index, duration)
	};

	struct texture_animation {
	private:
		bool	loop		= false;
		float	timer		= 0;
		int		frame_index = -1;
		string	name;
		std::vector<texture_frame> frames;

	public:
		texture_animation() : name("empty animation") {}
		texture_animation(string name, std::vector<texture_frame> frame_set) : name(name), frames(frame_set) {}

		void start			() { frame_index = 0; }
		void stop			() { frame_index = -1; }
		bool is_animating	() { return frame_index >= 0; }

		void set_loop(bool l) { loop = l; }

		bool update(float elapsed_time) {
			if (frame_index < 0) return false;				// Don't try and read negative indices
			timer += elapsed_time;							// Advance the timer of this frame
			while (timer > frames[frame_index].duration) {  // Keep advancing frames until the current frame duration exceeds the timer
				timer -= frames[frame_index++].duration;	// Decrememnt the timer by the current frame duration.
				if (frame_index >= frames.size()) {			// Check to see if the animation is over, and loop if necessary.
					if (loop) start();
					else { stop(); break; }
				}
			}
			return is_animating();
		}

		string get_name	() const { return name; }
		float2 get_frame() const { return frames[frame_index].uv_index; }

#ifdef _DEBUG
		auto& imgui_get_frames() { return frames; }
		void imgui_set_name(string s) { name = s; }
#endif

		SERIALIZE(frames)
	};

	class texture_animator {
	private:
		struct queued_animation { string name; bool loop = false; };

		std::unordered_map<string, texture_animation> animations;
		std::queue<queued_animation> animation_queue;

		// Get the current animation.
		texture_animation&			current()		{ return animations[animation_queue.front().name]; }
		// Get the current animation.
		const texture_animation&	current() const { return animations.at(animation_queue.front().name); }

		void update_loop() { current().set_loop(animation_queue.front().loop); }

	public:
		texture_animator() = default;

		// Add a new animation.
		void add_animation(texture_animation animation) { animations.try_emplace(animation.get_name(), animation); }

		// Check if an animation is running.
		bool is_animating() const { return animation_queue.size(); }

		// End the current animation and start the next. Returns False if there are no more animations queued.
		bool next() { animation_queue.pop(); return is_animating(); }

		// Stop the current animations and empty the queue.
		void stop_all() { if (is_animating()) while (next()); }

		// Set the current animation to loop (or not loop), if there is one.
		void set_loop(bool l) { if (is_animating()) { current().set_loop(l); } }

		/*
			Start animating.
			
			string name
				- The name of the animation to start.
			bool loop
				- Whether or not to loop the animation when it ends.
				- Optional. Defaults to false.
			bool overwrite
				- If true, stops all animations so that this one can start immediately.
				- Optional. Defaults to true.
		*/
		void animate(string animation_name, bool loop = false, bool overwrite = true) {
			bool was_animating = is_animating();
			if (overwrite) { stop_all(); }
			animation_queue.push({ animation_name, loop });
			if (!was_animating || overwrite) { // Basically, don't accidentally overwrite the last frame of the previous animation unless necessary.
				update_loop(); 
				current().start();
			}
		}

		void update(float elapsed_time) {
			if (!is_animating()) return; // Return if not animating.
			if (!current().update(elapsed_time)) { // If the current animation finished, start the next one.
				if (next()) { // If there is an update queued, start it.
					update_loop();
					current().start();
				}
			}
		}

		// Get the uv_index of the current animation, or {0, 0} if not animating.
		float2 get_frame() const { if (is_animating()) return current().get_frame(); else return float2(0); }

#ifdef _DEBUG
		auto& imgui_get_animations() { return animations; }
#endif

		SERIALIZE(animations)
	};

}