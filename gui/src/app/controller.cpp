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
	m_signals->refresh_root_directory.attach_to(m_slot, [this] { refresh_root_directory(); });

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

void Controller::on_drop(fs::path const& root) {
	if (!fs::is_directory(root)) { return; }
	m_main_window.scan_data.root = root;
	refresh_root_directory();
}

void Controller::shutdown() {
	// TODO: cancel async work.
	m_state = State::Finished;
}

void Controller::refresh_root_directory() {
	if (m_main_window.scan_data.root.empty()) {
		log.warn("attempt to refresh empty root directory");
		return;
	}

	if (!fs::is_directory(m_main_window.scan_data.root)) {
		log.warn("'{}' is not a directory, resetting", m_main_window.scan_data.root.generic_string());
		m_main_window.scan_data.root.clear();
		return;
	}

	auto const scan_info = to_scan_info(m_main_window.scan_data);
	auto result = m_instance->scan_directory(m_main_window.scan_data.root, scan_info);
	if (!result) {
		log.error("TODO: failed to load directory: '{}'", m_main_window.scan_data.root.generic_string());
		return;
	}

	m_main_window.refresh_root_directory(std::move(*result));
}
} // namespace xtag::gui
