#include "ui/main_window.hpp"
#include "app/log.hpp"
#include "klib/string/c_string.hpp"
#include <imgui.h>
#include <ranges>

namespace xtag::gui::ui {
namespace {
struct EntryData {
	std::string tree_uri{};
	std::string inspect_uri{};
};

auto to_data(Entry const& entry, fs::path const& root) -> EntryData {
	auto ret = EntryData{};

	auto const filename = entry.path.filename().string();
	auto const uri = fs::relative(entry.path, root).generic_string();
	if (entry.type == EntryType::Directory) {
		ret.inspect_uri = std::format("directory: {}", filename);
		ret.tree_uri = std::format("{}/", uri);
	} else {
		ret.inspect_uri = std::format("file: {}", filename);
		ret.tree_uri = uri;
	}

	return ret;
}

void populate_data(EntryList& list) {
	for (auto& entry : list.entries) { entry.custom_payload = to_data(entry, list.path); }
}
} // namespace

void MainWindow::update() {
	if (!m_file_browser) {
		ImGui::TextUnformatted("drag a directory here to begin");
		return;
	}

	ImGui::TextUnformatted(m_root_directory.c_str());

	auto& selected = m_file_browser->list.get_selected();
	auto& data = std::any_cast<EntryData&>(selected.custom_payload);

	auto refresh_root_directory = false;
	auto replace_tags = false;
	if (ImGui::BeginTable("top", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(data.inspect_uri.c_str());
		widget::TagEditor::update_short_tags(selected.tags);

		if (ImGui::Button("view tags")) { m_tag_editor.set_should_open(selected); }

		ImGui::TableNextColumn();
		scan_data.update();
		if (ImGui::Button("refresh")) { refresh_root_directory = true; }

		ImGui::EndTable();
	}

	m_file_browser->update_filter();
	m_file_browser->update_pagination();

	if (m_tag_editor.update() && should_replace_tags()) { replace_tags = true; }

	if (ImGui::BeginChild("list", {}, ImGuiChildFlags_Borders)) {
		update_current_page();
		ImGui::EndChild();
	}

	if (refresh_root_directory) {
		m_dispatch->refresh_root_directory();
	} else if (replace_tags) {
		m_dispatch->replace_tags(selected.path, m_tag_replacement);
	}
}

void MainWindow::set_list(EntryList list) {
	list.sort_entries();
	m_root_directory = list.path.generic_string();
	populate_data(list);
	if (!m_file_browser) {
		m_file_browser.emplace(std::move(list));
	} else {
		m_file_browser->list.refresh(std::move(list));
	}
	log.debug("Directory loaded successfully: '{}'", m_root_directory);
}

void MainWindow::update_current_page() {
	auto const page = m_file_browser->list.get_current_page();
	m_file_browser->update_number_width(page);
	for (auto const& [index, entry] : std::views::enumerate(page.entries)) {
		auto const& data = std::any_cast<EntryData const&>(entry->custom_payload);
		auto const number = int(index) + page.offset_from_start + 1;
		m_file_browser->update_entry(number, *entry, data.tree_uri);
	}
}

auto MainWindow::should_replace_tags() -> bool {
	auto const replacement = m_tag_editor.get_replacement();
	if (replacement.empty()) { return false; }

	m_tag_replacement.clear();
	m_tag_replacement.reserve(replacement.size());
	for (auto const& tag : replacement) { m_tag_replacement.push_back(tag); }

	return true;
}
} // namespace xtag::gui::ui
