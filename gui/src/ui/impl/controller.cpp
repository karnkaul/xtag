#include "ui/impl/controller.hpp"
#include "app/log.hpp"

namespace xtag::gui::ui {
namespace {
[[nodiscard]] auto to_scan_info(ui::ScanData const& data) -> ScanInfo {
	auto ret = ScanInfo{.depth = data.depth};
	ret.filter.include_files = data.include_files;
	ret.filter.tag_type |= TagType::Untagged;
	return ret;
}
} // namespace

void Controller::set_styles(ImGuiStyle& style) { style.CellPadding = {6.0f, 6.0f}; }

void Controller::initialize(Services const& services) {
	Object::initialize(services);

	m_instance = &services.get<Instance>();

	m_main_menu.initialize(services);
	m_main_window.initialize(services);

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
	auto& scan_data = m_main_window.scan_data;
	if (scan_data.root.empty()) {
		log.warn("attempt to refresh empty root directory");
		return;
	}

	if (!fs::is_directory(scan_data.root)) {
		log.warn("'{}' is not a directory, resetting", scan_data.root.generic_string());
		scan_data.root.clear();
		return;
	}

	auto const scan_info = to_scan_info(scan_data);
	auto result = m_instance->scan_directory(scan_data.root, scan_info);
	if (!result) {
		log.error("TODO: failed to load directory: '{}'", scan_data.root.generic_string());
		return;
	}

	if (result->entries.empty()) {
		log.error("TODO: internal error: EntryList is unexpectedly empty");
		return;
	}

	m_main_window.set_list(std::move(*result));
}
} // namespace xtag::gui::ui
