#pragma once
#include "string.h"
#define AUDIO_EXT L".wav"

namespace BLIB {

	enum audio_control {
		add, // Adds the value to the setting
		set, // Sets the setting to the value
		mlt, // Multiplies the setting by the value
	};

	enum audio_setting {
		volume,
		pitch,
		pan
	};

	namespace audio {

		enum {
			unset = -2,
			all_tracks = -1
		};

		void init();
		void update(float elapsed_time);
		void uninit();

		// Change the defualt local folder where audio are loaded from.
		void set_filepath(string path);

		// Load a .wav file.
		void load		(string filename);
		// Release cached audio data by name.
		void unload		(string filename = L"");

		// Play a track. Optionally fade in or loop. Returns the track id.
		int  play		(string filename, float fade_time = 0.0f, bool loop = false);
		// Pause a track by id. By default, pauses all tracks. Optionally, fade out into the pause.
		void pause		(int instance = all_tracks, float fade_time = 0.0f);
		// Resume a track by id. By default, resumes all tracks. Optionally, fade back in.
		void resume		(int instance = all_tracks, float fade_time = 0.0f);
		// Stop a track by id. By default, stops all tracks. Optionally, fade out.
		void stop		(int instance = all_tracks, float fade_time = 0.0f);

		// Checks if a track is looped by id. By default, checks to see if any track is looped.
		bool is_looped	(int instance = all_tracks);
		// Checks if a track is playing by id. By default, checks to see if any track is playing.
		bool is_playing	(int instance = all_tracks);
		// Checks if a track is paused by id. By default, checks to see if any track is paused.
		bool is_paused	(int instance = all_tracks);
		// Checks if a track is stopped by id. By default, checks to see if any track is stopped.
		bool is_stopped	(int instance = all_tracks);

		/*
			Edit the playback style of the track by name.This will change all future playbacks of the track.
			
			string filename
				- the name of the track
			audio_control op
				- the operation to perform on the setting.
			audio_setting target
				- the setting to change
			float val
				- the amound to change the setting by.
		*/
		void config(string filename, audio_control op, audio_setting target, float val);

	}

}