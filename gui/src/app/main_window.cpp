#include "app/main_window.hpp"
#include "app/log.hpp"
#include "klib/string/c_string.hpp"
#include "klib/string/fixed_string.hpp"
#include "xtag/formatter.hpp"
#include <imgui.h>
#include <array>
#include <ranges>

namespace xtag::gui {
namespace {
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

	auto const formatter = Formatter{};
	for (auto const& tag : entry.tags) { ret.all_tags.push_back(formatter.format(tag)); }
	formatter.truncate_to(ret.short_tags, ret.all_tags, 3);

	return ret;
}

void populate_data(EntryList& list) {
	for (auto& entry : list.entries) { entry.custom_payload = to_data(entry, list.path); }
}

[[nodiscard]] auto to_data(klib::Ptr<Entry const> selected) -> EntryData const& {
	static auto const s_default = EntryData{};
	if (!selected) { return s_default; }

	auto const* data = std::any_cast<EntryData>(&selected->custom_payload);
	if (!data) { return s_default; }

	return *data;
}

constexpr auto tag_viewer_label_v = klib::CString{"tag_viewer"};

void update_tag_viewer(EntryData const& entry) {
	if (!ImGui::BeginPopup(tag_viewer_label_v.c_str())) { return; }
	if (ImGui::BeginListBox("tags", {200.0f, 0.0f})) {
		for (auto const& tag : entry.all_tags) { ImGui::TextUnformatted(tag.c_str()); }
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

	auto const& entry = to_data(m_file_browser->get_selected());

	if (ImGui::BeginTable("top", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		update_inspector(entry);

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
	update_tag_viewer(entry);

	if (ImGui::BeginChild("list", {}, ImGuiChildFlags_Borders)) {
		update_browser();
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
	// ImGui::SameLine();
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

void MainWindow::update_inspector(EntryData const& entry) {
	ImGui::TextUnformatted(entry.inspect_uri.c_str());
	ImGui::TextUnformatted(entry.short_tags.c_str());
	if (ImGui::Button("view tags")) { m_open_tag_viewer = true; }
}

void MainWindow::update_browser() {
	KLIB_ASSERT(m_file_browser);

	auto const selected = m_file_browser->get_selected();
	auto const entries = m_file_browser->get_current_page();
	auto const number_width = [&] {
		auto ret = 1;
		for (auto count = entries.size(); count >= 10; count /= 10) { ++ret; }
		return ret;
	}();
	for (auto const& [index, entry] : std::views::enumerate(entries)) {
		auto const& data = to_data(entry);
		auto const number = index + 1;
		ImGui::TextUnformatted(klib::FixedString{"{: >{}}.", number, number_width}.c_str());
		ImGui::SameLine();
		if (ImGui::Selectable(data.tree_uri.c_str(), entry == selected)) { m_file_browser->select_entry(*entry); }
	}
}
} // namespace xtag::gui
