#pragma once
#include "verify.h"
#include <thread>
#include <atomic>
#include <windows.h>
#include <set>
#include "manager.h"

/*
	
	The status is the building block of how things get done.

	They are handled by the manager and each have a unique ID.

	They automatically load themselves asynchronously when first ticked.
	After they load, they automatically wake themselves.

	Each status has a built in timer. It's a lifetime timer by default,
	but it can be reset for whatever purpose you may need.

	A status can be preserved if you don't want it to be deleted, even
	when it's finished. This is typically done by "signing" the status
	with a pointer to the object preserving it. If you forget to 
	unpreserve a status, it won't be deleted until the program ends.

	When making your own task, you should override the following:

	init()
		All initialization should be done here, not the constructor, 
		whenever possible.

	update()
		Called every frame when awake.

	idle()
		Called every frame when asleep.

	kill()
		Called just before deletion. This is the chance to stop 
		processes that depend on this status or release resources.


*/

namespace BLIB {

	class status {
	public:
		enum activity {
			unloaded,	// This status is not ready.
			inactive,	// This status is calling idle() every frame
			active,		// This status is calling update() every frame
			stopping,	// This status is calling idle() every frame, and is preparing to finish.
			finished	// This status is calling idle() every frame, and is ready to be deleted.
		};

	private:
		std::thread*		loader = nullptr;
		std::atomic<bool>	loaded { false };
		std::set<void*>		preservatives{};

		task_id		id			= 0;
		activity	state		= unloaded;

	protected:
		float		timer		= 0;

		virtual void	init		() {}
		void			coinit		() { srand(static_cast<unsigned int>(time(nullptr))); HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED); VERIFY; init(); CoUninitialize(); loaded.store(true, std::memory_order_release); }
		void			finish		() { state = finished; }

		void force_wake	() { state = active;	on_wake	();	}
		void force_sleep() { state = inactive;	on_sleep(); }
		void force_stop	() { state = stopping;  on_stop (); }

		virtual void	on_load		() { force_wake(); }
		virtual void	on_wake		() {}
		virtual void	on_sleep	() {}
		virtual void	on_stop		() {}

		virtual void	update		(float elapsed_time)	{ finish(); }
		virtual void	idle		(float elapsed_time)	{			}
		virtual void	try_stop	()						{ finish(); }

	public:
		virtual ~status() { preservatives.clear(); if (loader) { while (!loader->joinable()); loader->join(); delete loader; } }//{ _ASSERT_EXPR(loader == nullptr, L"Status deleted improperly"); }

		// Check if loading is done yet.
		void try_end_load() {
			if (!loader) return;													// wdym I'm not loading
			if (loader->joinable() && loaded.load(std::memory_order_acquire)) {		// Check if init thread has finished
				loader->join();														// Join the thread
				on_load();															// Awake (or sleep, depening on behavior)
				delete loader;														// Delete init thread
				loader = nullptr;													// Clear the pointer
			}
		}

		inline virtual void tick(float elapsed_time) { 
			timer += elapsed_time;																							// Advance the timer
			switch (state) {																					
			case unloaded:	if (loader) { try_end_load(); }																	// If unloaded and loading, check if done
							else if (!loader) {loader = new std::thread(&status::coinit, this); }		break;				// Otherwise, if unloaded, spin up init thread
			case active:	update(elapsed_time);														break;				// If active, update
			case stopping:  try_stop();																	[[fallthrough]];	// If stopping, try to stop, and then idle
			default:		idle(elapsed_time);															break;				// Otherwise, idle
			}
		};

		void			wake	()	{ if (state != inactive)					{ return; } force_wake	();	}
		void			sleep	()	{ if (state != active)						{ return; } force_sleep	();	}
		void			stop	()	{ if (state != inactive && state != active)	{ return; }	force_stop	(); }
		virtual void	kill	()	{} // Ubruptly end the task because I'm about to delete you

		activity	report		() const { return state; }
		float		get_time	() const { return timer; }

		// Sign this status to prevent it from being deleted
		void preserve		(void* p)	{ preservatives.insert(p);			} 
		// Unsign this status to allow it to be deleted
		void unpreserve		(void* p)	{ preservatives.erase(p);			}
		// Check if this status has been signed
		bool is_preserved	() const	{ return preservatives.size() > 0;	}

		task_id get_id	() const		{ return id; }
		bool	init_id	(task_id tid)	{ if (!id) { id = tid; return true; } else { return false; } }
	};

}