#include "app/controller.hpp"
#include <imgui.h>

namespace xtag::gui {
void Controller::initialize(Services const& services) {
	Object::initialize(services);

	m_main_menu.initialize(services);
	m_main_window.initialize(services);
}

void Controller::update() {
	if (ImGui::BeginMainMenuBar()) {
		m_main_menu.update();
		ImGui::EndMainMenuBar();
	}

	auto const& viewport = *ImGui::GetMainViewport();
	ImGui::SetNextWindowPos({0.0f, viewport.WorkPos.y});
	ImGui::SetNextWindowSize(viewport.WorkSize);
	ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

	m_main_window.update();

	ImGui::End();
}
} // namespace xtag::gui
