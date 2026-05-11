#include "app/main_menu.hpp"
#include <imgui.h>

namespace xtag::gui {
void MainMenu::update() {
	if (ImGui::BeginMenu("File")) {
		ImGui::Separator();
		if (ImGui::MenuItem("Quit")) { m_signals->shutdown.dispatch(); }
		ImGui::EndMenu();
	}
}
} // namespace xtag::gui
