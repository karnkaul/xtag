#include "ui/controller.hpp"
#include "app/log.hpp"
#include "ui/entry_data.hpp"

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

	m_main_window.initialize(services);
	m_loading_modal.initialize(services);

	m_state = State::Running;
}

void Controller::update() {
	auto const& viewport = *ImGui::GetMainViewport();
	ImGui::SetNextWindowPos({0.0f, viewport.WorkPos.y});
	ImGui::SetNextWindowSize(viewport.WorkSize);
	ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

	m_main_window.update();
	m_loading_modal.update();
	poll_future();

	ImGui::End();

	if (m_future.valid()) { return; }

	switch (m_main_window.get_action()) {
	case Action::None: return;
	case Action::RefreshRoot: refresh_root_directory(); break;
	case Action::ReplaceTags: replace_tags(); break;
	}
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
	KLIB_ASSERT(!m_future.valid());

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

void Controller::replace_tags() {
	KLIB_ASSERT(!m_future.valid());

	auto const selected = m_main_window.get_selected();
	if (!selected || selected->path.empty()) { return; }

	auto const replacement_tags = m_main_window.get_replacement_tags();
	auto const result = [&] {
		if (replacement_tags.empty()) { return m_instance->erase_tags(selected->path); }
		return m_instance->replace_tags(selected->path, replacement_tags);
	}();

	if (!result) {
		log.error("TODO: failed to replace tags on '{}': {}", selected->path.generic_string(), result.error().message);
		return;
	}

	log.info("tags replaced successfully: '{}', refreshing root directory", selected->path.generic_string());
	refresh_root_directory();
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

	m_entry_list = std::make_shared<EntryList>(std::move(*result));

	m_entry_list->sort_entries();
	for (auto& entry : m_entry_list->entries) { EntryData::write_to(entry, m_entry_list->path); }

	m_main_window.set_list(m_entry_list);
	m_loading_modal.set_should_close();
}
} // namespace xtag::gui::ui
