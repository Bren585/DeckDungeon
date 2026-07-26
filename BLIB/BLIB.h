#pragma once

#include "verify.h"
#include "manager.h"
#include "blend.h"
#include "rasterize.h"
#include "depth_stencil.h"
#include "sampler.h"
#include "math.h"
#include "window.h"
#include "lighting.h"
#include "input.h"
#include "text.h"
#include "entity.h"
#include "render_lock.h"
#include "audio.h"
#include "render_target.h"

namespace BLIB {

	// Start the application
	bool	init		(_In_ HINSTANCE instance, _In_opt_  HINSTANCE prev_instance, _In_ LPSTR cmd_line, _In_ int cmd_show, const wchar_t* name = L"BLIB");
	// Destroy the application
	int		uninit		();

	// Called in main. Returns false once the application is finished.
	bool	loop		();

	void	update		();
	void	render		();

	// The time taken in between frames. AKA Delta Time. 
	float	dtime		();
	bool	is_active	();

	// Debug tool for framerate analysis
	static void				calculate_frame_stats	();

	// Handle input from Windows API
	static LRESULT CALLBACK handle_message			(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}