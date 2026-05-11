#include "app/main_window.hpp"
#include "klib/string/c_string.hpp"
#include <imgui.h>

namespace xtag::gui {
namespace {
template <klib::EnumT T>
void flag_to_checkbox(klib::CString const label, T& mask, T const enumeration) {
	auto enabled = (mask & enumeration) == enumeration;
	if (ImGui::Checkbox(label.c_str(), &enabled)) {
		if (enabled) {
			mask |= enumeration;
		} else {
			mask &= ~enumeration;
		}
	}
}
} // namespace

void MainWindow::update() {
	update_model();

	if (ImGui::BeginChild("list", {}, ImGuiChildFlags_Borders)) {
		ImGui::TextUnformatted("child window 1");
		ImGui::TextUnformatted("child window 2");
		ImGui::EndChild();
	}
}

void MainWindow::update_model() {
	ImGui::TextUnformatted("entry type");
	for (auto const& [type, name] : entry_type_name_map.as_span()) {
		if (type == EntryType::None) { continue; }
		ImGui::SameLine();
		flag_to_checkbox(name.data(), m_shared_model->scan_data.entry_type, type);
	}

	ImGui::SetNextItemWidth(100.0f);
	ImGui::DragInt("scan depth", &m_shared_model->scan_data.depth, 1.0f, 0, 100);
}
} // namespace xtag::gui
