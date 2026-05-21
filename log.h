#pragma once
#include <vector>
#include "BLIB/string.h"

#define MAX_BACKLOG 30

namespace event_log {
	void record(string message);
	void clear();
	void imgui();
	const std::vector<string>& get();
};