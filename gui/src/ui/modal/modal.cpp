#include "ui/modal/modal.hpp"
#include <imgui.h>

namespace xtag::gui::ui {
auto Modal::is_open() const -> bool {
	if (label.as_view().empty()) { return false; }
	return ImGui::IsPopupOpen(label.c_str());
}

void Modal::set_should_open(bool const should_open) {
	if (label.as_view().empty()) { return; }
	m_should_open = should_open;
}

void Modal::set_should_close(bool const should_close) { m_should_close = should_close; }

void Modal::update(Seconds const dt) {
	if (m_should_open) {
		m_should_open = false;
		ImGui::OpenPopup(label.c_str());
	}

	auto const center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
	if (!ImGui::BeginPopupModal(label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) { return; }
	on_update(dt);
	if (m_should_close) { ImGui::CloseCurrentPopup(); }
	ImGui::EndPopup();
}

void Modal::on_update(Seconds const /*dt*/) {
	ImGui::TextUnformatted("<Add content here>");
	ImGui::Separator();
	if (ImGui::Button("Close")) { set_should_close(); }
}
} // namespace xtag::gui::ui
