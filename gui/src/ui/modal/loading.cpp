#include "ui/modal/loading.hpp"
#include <imgui.h>

namespace xtag::gui::ui {
void LoadingModal::on_open() {
	m_dot_count = 1;
	m_dot_remain = dot_rate;
	refresh_dots();
}

void LoadingModal::on_update() {
	m_dot_remain -= m_delta_time->get_dt();
	if (m_dot_remain <= 0s) {
		m_dot_remain = dot_rate;
		bump_dots();
	}

	ImGui::TextUnformatted(body_text.c_str());
	ImGui::TextUnformatted(m_dots.c_str());
}

void LoadingModal::bump_dots() {
	m_dot_count = (m_dot_count + 1) % max_dots;
	refresh_dots();
}

void LoadingModal::refresh_dots() {
	m_dots.clear();
	m_dots.reserve(std::size_t(m_dot_count));
	for (int i = 0; i < m_dot_count; ++i) { m_dots.push_back('.'); }
}
} // namespace xtag::gui::ui
