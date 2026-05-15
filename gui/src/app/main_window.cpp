#include "app/main_window.hpp"
#include "app/log.hpp"
#include "klib/string/c_string.hpp"
#include "klib/string/fixed_string.hpp"
#include <imgui.h>
#include <array>
#include <ranges>

namespace xtag::gui {
namespace {
struct EntryData {
	std::string tree_uri{};
	std::string inspect_uri{};
	std::vector<std::string> all_tags{};
};

constexpr auto page_limits_v = std::array{10, 20, 50, 99};

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

constexpr auto tag_viewer_label_v = klib::CString{"tag_viewer"};

constexpr auto get_tag_color(TagType const type) {
	switch (type) {
	case TagType::Primary: return ImVec4{0.0f, 0.8f, 1.0f, 1.0f};
	case TagType::Inherited: return ImVec4{0.8f, 0.8f, 0.0f, 1.0f};
	default: return ImVec4{0.5f, 0.5f, 0.5f, 1.0f};
	}
}

void tag_text_colored(ScanTag const& tag) {
	auto const color = get_tag_color(tag.type);
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	ImGui::TextColored(color, "%s", tag.value.data());
}

void update_tag_viewer(Entry const& entry) {
	if (!ImGui::BeginPopup(tag_viewer_label_v.c_str())) { return; }
	if (ImGui::BeginListBox("tags", {200.0f, 0.0f})) {
		for (auto const& tag : entry.tags) { tag_text_colored(tag); }
		ImGui::EndListBox();
	}
	ImGui::EndPopup();
}
} // namespace

void MainWindow::update() {
	if (!m_file_browser) {
		ImGui::TextUnformatted("drag a directory here to begin");
		return;
	}

	auto const& selected = m_file_browser->get_selected();

	if (ImGui::BeginTable("top", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		update_inspector(selected);

		ImGui::TableNextColumn();
		update_scan_data();

		ImGui::EndTable();
	}

	update_filters();
	update_pagination();

	if (m_open_tag_viewer) {
		m_open_tag_viewer = false;
		ImGui::OpenPopup(tag_viewer_label_v.c_str());
	}
	update_tag_viewer(selected);

	if (ImGui::BeginChild("list", {}, ImGuiChildFlags_Borders)) {
		update_list(selected);
		ImGui::EndChild();
	}
}

void MainWindow::set_list(EntryList list) {
	list.sort_entries();
	m_root_directory = list.path.generic_string();
	populate_data(list);
	if (!m_file_browser) {
		m_file_browser.emplace(std::move(list), page_limits_v.back());
	} else {
		m_file_browser->refresh(std::move(list));
	}
	log.debug("Directory loaded successfully: '{}'", m_root_directory);
}

void MainWindow::update_scan_data() {
	ImGui::Checkbox("include files", &scan_data.include_files);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.0f);
	ImGui::DragInt("scan depth", &scan_data.depth, 1.0f, 0, 100);
	if (ImGui::Button("refresh")) { m_signals->refresh_root_directory.dispatch(); }
}

void MainWindow::update_filters() {
	ImGui::TextUnformatted("list filters:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputText("include", m_filter.allow.data(), m_filter.allow.size());
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputText("exclude", m_filter.block.data(), m_filter.block.size());
	ImGui::SameLine();
	if (ImGui::Button("apply")) { m_file_browser->apply_filter(m_filter.allow.data(), m_filter.block.data()); }
}

void MainWindow::update_pagination() {
	auto const page_number = m_file_browser->get_page_number();
	auto const page_count = m_file_browser->get_page_count();

	ImGui::BeginDisabled(page_number == 0);
	if (ImGui::Button("<<")) { m_file_browser->set_page_number(page_number - 1); }
	ImGui::EndDisabled();
	ImGui::SameLine();

	ImGui::BeginDisabled(page_count == 0);
	ImGui::SetNextItemWidth(50.0f);
	if (ImGui::BeginCombo("##page", klib::FixedString{"{}", page_number}.c_str())) {
		for (int number = 0; number < page_count; ++number) {
			if (ImGui::Selectable(klib::FixedString{"{}", number}.c_str(), number == page_number)) { m_file_browser->set_page_number(number); }
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(page_number + 1 == page_count);
	if (ImGui::Button(">>")) { m_file_browser->set_page_number(page_number + 1); }
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::BeginCombo("items per page", klib::FixedString<>{"{}", m_file_browser->get_page_limit()}.c_str())) {
		auto const page_limit = [this](int const limit) {
			if (ImGui::Selectable(klib::FixedString{"{}", limit}.c_str(), m_file_browser->get_page_limit() == limit)) { m_file_browser->repaginate(limit); }
		};
		for (auto const limit : page_limits_v) { page_limit(limit); }
		ImGui::EndCombo();
	}
}

void MainWindow::update_inspector(Entry const& selected) {
	static constexpr auto max_tags_v{3};
	auto const& data = std::any_cast<EntryData const&>(selected.custom_payload);
	ImGui::TextUnformatted(data.inspect_uri.c_str());
	for (auto const& [index, tag] : std::views::enumerate(selected.tags)) {
		if (index > 0) {
			ImGui::SameLine();
			ImGui::TextUnformatted("|");
			ImGui::SameLine();
		}
		if (index >= max_tags_v) {
			ImGui::TextUnformatted("...");
			break;
		}
		tag_text_colored(tag);
	}
	if (ImGui::Button("view tags")) { m_open_tag_viewer = true; }
}

void MainWindow::update_list(Entry const& selected) {
	auto const page = m_file_browser->get_current_page();
	auto const number_width = [&] {
		auto ret = 1;
		for (auto count = page.size(); count >= 10; count /= 10) { ++ret; }
		return ret;
	}();
	for (auto const& [index, entry] : std::views::enumerate(page)) {
		auto const& data = std::any_cast<EntryData const&>(entry->custom_payload);
		auto const number = index + 1;
		ImGui::TextUnformatted(klib::FixedString{"{: >{}}.", number, number_width}.c_str());
		ImGui::SameLine();
		if (ImGui::Selectable(data.tree_uri.c_str(), entry == &selected)) { m_file_browser->select_entry(*entry); }
	}
}
} // namespace xtag::gui
