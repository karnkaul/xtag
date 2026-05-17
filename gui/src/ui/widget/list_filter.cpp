#include "ui/widget/list_filter.hpp"

namespace xtag::gui::ui::widget {
void ListFilter::update() {
	ImGui::TextUnformatted("list filters:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150.0f);
	allow.update("include");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150.0f);
	block.update("exclude");
	ImGui::SameLine();
}
} // namespace xtag::gui::ui::widget
