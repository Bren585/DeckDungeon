#include "log.h"
#include "BLIB/imgui/imgui.h"

std::vector<string> log_data;

void event_log::record(string message) { log_data.push_back(message); }

void event_log::imgui() {
	if (ImGui::Begin("Debug Log", nullptr, 0)) {
		int log_count = 0;
		int back_up = (int) ((MAX_BACKLOG > log_data.size()) ? log_data.size() : MAX_BACKLOG);
		for (auto it = log_data.end() - back_up; it != log_data.end() && log_count < MAX_BACKLOG; it++) {
			ImGui::Text(*it);
			log_count++;
		}
	}
	ImGui::End();
}

const std::vector<string>& event_log::get() { return log_data; }

void event_log::clear() { log_data.clear(); }