#include "app/main_window.hpp"
#include <imgui.h>

namespace xtag::gui {
void MainWindow::update() {
	ImGui::TextUnformatted("main window");

	if (ImGui::BeginChild("list", {}, ImGuiChildFlags_Borders)) {
		ImGui::TextUnformatted("child window 1");
		ImGui::TextUnformatted("child window 2");
		ImGui::EndChild();
	}
}
} // namespace xtag::gui
