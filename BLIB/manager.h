#pragma once
#include "BLIB.h"
#include "math.h"

namespace BLIB {

	class status;
	namespace generic { class scene; }

	typedef unsigned int task_id;

	enum class transition {
		none,
		fade
	};

	namespace manager {

		namespace _private {
			std::map<task_id, std::unique_ptr<status>>& get_tasks();
		}

		constexpr int scene_stack_size = 4;

		// Tick all tasks. Returns false is there are no unfinished tasks.
		bool tick	(float elapsed_time);
		// Resize all staged scenes.
		void resize	(float2 size);
		void kill	(); // End immedietly, cleanup
		//void end	(); // Exit process

		//task_id	add(generic::scene* s);

		// Register a new task.
		task_id		add				(status* s);

		// Stage a scene to be displayed to the screen.
		// Multiple scenes can be stages at once using multiple slots.
		// Slots in higher numbers are displayed behind lower numbers.
		// Optionally, the scene can transitioned into.
		void		stage			(task_id id, int slot, transition t = transition::none, float duration = 0.0f);

		// Register a new scene and stage it to be displayed to the screen.
		// Multiple scenes can be stages at once using multiple slots.
		// Slots in higher numbers are displayed behind lower numbers.
		// Optionally, the scene can transitioned into.
		task_id		add_and_stage	(generic::scene* s, int slot, transition t = transition::none, float duration = 0.0f);

		// Remove a scene from the stage.
		// Optionally, the scene can transitioned out.
		int			unstage			(task_id id, transition t = transition::none, float duration = 0.0f);
		
		// Returns the task_id of the first task of the given type.
		template <class S>
		task_id find_first_of_type() {
			for (auto& pair : _private::get_tasks()) {
				if (dynamic_cast<S*>(pair.second.get())) {
					return pair.first;
				}
			}
			return 0;
		}

		// Render the staged scenes to the screen, from back to front.
		void display();

		// Get a task by its ID.
		status*							get_task	(task_id id);
		// Get a scene by its ID.
		generic::scene*					get_scene	(task_id id);
		// Get the scene in the given slot.
		generic::scene*					get_slot	(int slot);

		inline const status*			peek_task	(task_id id)	{ return get_task(id); }
		inline const generic::scene*	peek_scene	(task_id id)	{ return get_scene(id); }
		inline const generic::scene*	peek_slot	(int slot)		{ return get_slot(slot); }

	}

}