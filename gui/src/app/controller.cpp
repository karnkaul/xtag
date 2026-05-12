#include "app/controller.hpp"
#include "app/log.hpp"
#include <imgui.h>

namespace xtag::gui {
namespace {
[[nodiscard]] auto to_scan_info(ScanData const& data) -> ScanInfo {
	auto ret = ScanInfo{.depth = data.depth};
	ret.filter.include_files = data.include_files;
	ret.filter.tag_type |= TagType::Untagged;
	return ret;
}
} // namespace

void Controller::initialize(Services const& services) {
	Object::initialize(services);

	m_instance = &services.get<Instance>();

	m_main_menu.initialize(services);
	m_main_window.initialize(services);

	m_signals->shutdown.attach_to(m_slot, [this] { shutdown(); });

	m_state = State::Running;
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

void Controller::on_drop(fs::path const& path) {
	if (!fs::is_directory(path)) { return; }

	log.debug("TODO: handle directory drop");
}

void Controller::shutdown() {
	// TODO: cancel async work.
	m_state = State::Finished;
}
} // namespace xtag::gui
