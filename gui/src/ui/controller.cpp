#include "ui/controller.hpp"
#include "app/log.hpp"

namespace xtag::gui::ui {
namespace {
[[nodiscard]] auto to_scan_info(ui::widget::ScanData const& data) -> ScanInfo {
	auto ret = ScanInfo{.depth = data.depth};
	ret.filter.include_files = data.include_files;
	ret.filter.tag_type |= TagType::Untagged;
	return ret;
}

template <typename T>
[[nodiscard]] auto is_ready(std::future<T> const& future) {
	return future.valid() && future.wait_for(0s) == std::future_status::ready;
}
} // namespace

void Controller::set_styles(ImGuiStyle& style) { style.CellPadding = {6.0f, 6.0f}; }

void Controller::initialize(Services const& services) {
	Object::initialize(services);

	m_instance = &services.get<Instance>();
	m_delta_time = &services.get<DeltaTime>();

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
	m_loading_modal.update(m_delta_time->get_dt());
	poll_future();

	if constexpr (klib::debug_v) { m_test_modal.update(m_delta_time->get_dt()); }

	ImGui::End();
}

void Controller::on_drop(fs::path const& root) {
	if (!fs::is_directory(root)) { return; }
	m_root = root;
	refresh_root_directory();
}

void Controller::shutdown() {
	if (m_future.valid()) { m_future.wait(); }
	m_state = State::Finished;
}

void Controller::refresh_root_directory() {
	if (m_root.empty()) {
		log.warn("attempt to refresh empty root directory");
		return;
	}

	if (!fs::is_directory(m_root)) {
		log.warn("'{}' is not a directory, resetting", m_root.generic_string());
		m_root.clear();
		return;
	}

	auto const scan_info = to_scan_info(m_main_window.scan_data);
	m_future = std::async([this, scan_info] { return m_instance->scan_directory(m_root, scan_info); });
	m_loading_modal.set_should_open();
}

void Controller::open_test_modal() {
	if constexpr (klib::debug_v) { m_test_modal.set_should_open(); }
}

void Controller::poll_future() {
	if (!is_ready(m_future)) { return; }

	auto result = m_future.get();
	if (!result) {
		log.error("TODO: failed to load directory: '{}'", m_root.generic_string());
		return;
	}

	if (result->entries.empty()) {
		log.error("TODO: internal error: EntryList is unexpectedly empty");
		return;
	}

	m_main_window.set_list(std::move(*result));
	m_loading_modal.set_should_close();
}
} // namespace xtag::gui::ui
