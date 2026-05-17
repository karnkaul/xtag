#include "ui/main_menu.hpp"
#include "klib/constants.hpp"
#include <imgui.h>

namespace xtag::gui::ui {
void MainMenu::update() {
	if (ImGui::BeginMenu("File")) {
		ImGui::Separator();
		if (ImGui::MenuItem("Quit")) { m_dispatch->shutdown(); }
		ImGui::EndMenu();
	}

	if constexpr (klib::debug_v) {
		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("Test modal")) { m_dispatch->open_test_modal(); }
			ImGui::EndMenu();
		}
	}
}
} // namespace xtag::gui::ui
