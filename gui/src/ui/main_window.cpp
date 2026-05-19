#include "ui/main_window.hpp"
#include "app/log.hpp"
#include "ui/entry_data.hpp"
#include "xtag/query.hpp"
#include <imgui.h>

namespace xtag::gui::ui {
namespace {
constexpr auto tag_editor_label_v = klib::CString{"tag_editor"};
} // namespace

void MainWindow::update() {
	m_action = Action::None;

	update_header();
	update_table();
	update_controls();
	update_current_page();
	update_tag_editor();

	switch (m_action) {
	case Action::None: return;
	case Action::RefreshRoot: m_dispatch->refresh_root_directory(); break;
	case Action::ReplaceTags: {
		if (m_file_browser.file_list) { m_dispatch->replace_tags(m_file_browser.file_list->get_selected().path, m_tag_replacement); }
		break;
	}
	}
}

void MainWindow::set_list(std::shared_ptr<EntryList const> list) {
	m_root_directory = list->path.generic_string();
	if (!m_file_browser.file_list) {
		m_file_browser.file_list.emplace(std::move(list), widget::FileBrowser::default_page_limit_v);
	} else {
		m_file_browser.file_list->refresh(std::move(list));
	}
	log.debug("Directory loaded successfully: '{}'", m_root_directory);
}

void MainWindow::set_filter(std::string_view const query) {
	if (!m_file_browser.file_list) { return; }

	if (query.empty()) {
		m_file_browser.file_list->clear_filter();
	} else {
		auto const expression = query::parse(query);
		auto const should_include = [&](Entry const& entry) {
			auto const& data = EntryData::read_from(entry);
			return expression.is_match(data.relative_path, entry.tags);
		};
		m_file_browser.file_list->apply_filter(should_include);
	}
}

void MainWindow::update_header() {
	if (m_root_directory.empty()) {
		ImGui::TextUnformatted("drag a directory here to begin");
	} else {
		ImGui::TextUnformatted(m_root_directory.c_str());
	}
}

void MainWindow::update_table() {
	if (ImGui::BeginTable("top", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedSame)) {
		ImGui::TableSetupColumn("left", ImGuiTableColumnFlags_WidthFixed, 200.0f);
		ImGui::TableSetupColumn("right", ImGuiTableColumnFlags_WidthStretch, 2.0f);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		scan_data.update();
		ImGui::BeginDisabled(!m_file_browser.file_list);
		if (ImGui::Button("refresh")) { m_action = Action::RefreshRoot; }
		ImGui::EndDisabled();

		ImGui::TableNextColumn();
		if (m_file_browser.file_list) {
			auto const& selected = m_file_browser.file_list->get_selected();
			if (selected.type == EntryType::Directory) {
				ImGui::TextUnformatted("directory:");
			} else {
				ImGui::TextUnformatted("file:");
			}

			auto const& data = EntryData::read_from(selected);
			ImGui::SameLine();
			ImGui::TextUnformatted(data.relative_path.c_str());
			widget::TagEditor::update_short_tags(selected.tags);

			if (ImGui::Button("edit tags")) {
				m_tag_editor.extract_tags(selected);
				m_open_tag_editor = true;
			}
		}

		ImGui::EndTable();
	}
}

void MainWindow::update_controls() {
	auto query = std::string_view{};
	if (m_file_browser.update_filter(query)) { set_filter(query); }
	m_file_browser.update_pagination();
}

void MainWindow::update_current_page() {
	if (!ImGui::BeginChild("current_page", {}, ImGuiChildFlags_Borders)) { return; }
	m_file_browser.update_current_page();
	ImGui::EndChild();
}

void MainWindow::update_tag_editor() {
	if (m_open_tag_editor) {
		m_open_tag_editor = false;
		ImGui::OpenPopup(tag_editor_label_v.c_str());
	}

	if (!ImGui::BeginPopup(tag_editor_label_v.c_str())) { return; }

	auto const tags_changed = m_tag_editor.update();
	ImGui::EndPopup();

	if (!tags_changed) { return; }

	set_tag_replacement();
	m_action = Action::ReplaceTags;
}

void MainWindow::set_tag_replacement() {
	m_tag_replacement.clear();
	auto const replacement = m_tag_editor.get_replacement();
	if (replacement.empty()) { return; }

	m_tag_replacement.reserve(replacement.size());
	for (auto const& tag : replacement) { m_tag_replacement.push_back(tag); }
}
} // namespace xtag::gui::ui
