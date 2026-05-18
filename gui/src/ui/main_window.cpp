#include "ui/main_window.hpp"
#include "app/log.hpp"
#include "klib/string/c_string.hpp"
#include "xtag/query.hpp"
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
	auto uri = fs::relative(entry.path, root).generic_string();
	if (entry.type == EntryType::Directory) {
		ret.inspect_uri = std::format("directory: {}", filename);
		ret.tree_uri = std::format("{}/", uri);
	} else {
		ret.inspect_uri = std::format("file: {}", filename);
		ret.tree_uri = std::move(uri);
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

	auto const& selected = m_file_browser->list.get_selected();
	auto const& data = std::any_cast<EntryData const&>(selected.custom_payload);

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

	auto query = std::string_view{};
	if (m_file_browser->update_filter(query)) { set_filter(query); }
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

void MainWindow::set_filter(std::string_view const query) {
	if (query.empty()) {
		m_file_browser->list.clear_filter();
	} else {
		auto const expression = query::parse(query);
		auto const should_include = [&](Entry const& entry) {
			auto const& data = std::any_cast<EntryData const&>(entry.custom_payload);
			return expression.is_match(data.tree_uri, entry.tags);
		};
		m_file_browser->list.apply_filter(should_include);
	}
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
