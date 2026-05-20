#include "ui/widget/scan_data.hpp"
#include <imgui.h>

namespace xtag::gui::ui::widget {
void ScanData::update() {
	ImGui::Checkbox("include files", &include_files);
	ImGui::SetNextItemWidth(60.0f);
	ImGui::DragInt("scan depth", &depth, 1.0f, 0, 100);
}
} // namespace xtag::gui::ui::widget
