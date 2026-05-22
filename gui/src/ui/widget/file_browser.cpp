#include "ui/widget/file_browser.hpp"
#include "klib/string/fixed_string.hpp"
#include "ui/entry_data.hpp"
#include <ranges>

namespace xtag::gui::ui::widget {
namespace {
auto update_page_number(int const page_number, int const page_count) -> int {
	auto ret = page_number;

	ImGui::BeginDisabled(page_number <= 0);
	if (ImGui::Button("<<")) { ret = page_number - 1; }
	ImGui::EndDisabled();
	ImGui::SameLine();

	ImGui::BeginDisabled(page_count == 0);
	ImGui::SetNextItemWidth(50.0f);
	if (ImGui::BeginCombo("##page", klib::FixedString{"{}", page_number}.c_str())) {
		for (int number = 0; number < page_count; ++number) {
			if (ImGui::Selectable(klib::FixedString{"{}", number}.c_str(), number == page_number)) { ret = number; }
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(page_number + 1 >= page_count);
	if (ImGui::Button(">>")) { ret = page_number + 1; }
	ImGui::EndDisabled();

	return ret;
}

auto update_page_size(int const page_limit) -> int {
	ImGui::SetNextItemWidth(80.0f);
	auto ret = page_limit;
	if (ImGui::BeginCombo("items per page", klib::FixedString<>{"{}", page_limit}.c_str())) {
		auto const update_limit = [&](int const limit) {
			if (ImGui::Selectable(klib::FixedString{"{}", limit}.c_str(), page_limit == limit)) { ret = limit; }
		};
		for (auto const limit : FileBrowser::page_limits_v) { update_limit(limit); }
		ImGui::EndCombo();
	}
	return ret;
}
} // namespace

void FileBrowser::update_pagination() {
	if (!file_list) {
		update_page_number(0, 0);
		ImGui::SameLine();
		ImGui::TextUnformatted("0-0 / 0");
		ImGui::SameLine();
		ImGui::BeginDisabled();
		update_page_size(default_page_limit_v);
		ImGui::EndDisabled();
		return;
	}

	auto const page_number = file_list->get_page_number();
	auto const new_page_number = update_page_number(page_number, file_list->get_page_count());
	if (new_page_number != page_number) { file_list->set_page_number(new_page_number); }

	auto const page = file_list->get_current_page();
	auto const start_number = page.offset_from_start + 1;
	auto const end_number = start_number + int(page.entries.size()) - 1;
	auto const filtered_count = int(file_list->get_filtered().size());
	ImGui::SameLine();
	ImGui::TextUnformatted(klib::FixedString{"{}-{} / {}", start_number, end_number, filtered_count}.c_str());

	ImGui::SameLine();
	auto const page_limit = file_list->get_page_limit();
	auto const new_page_limit = update_page_size(page_limit);
	if (new_page_limit != page_limit) { file_list->repaginate(new_page_limit); }
}

auto FileBrowser::update_filter() -> bool {
	ImGui::SetNextItemWidth(150.0f);
	static constexpr auto flags_v = ImGuiInputTextFlags_EnterReturnsTrue;
	auto ret = m_query_input.update("query", flags_v);
	ImGui::SameLine();
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	ImGui::TextColored({0.5f, 0.5f, 0.5f, 1.0f}, "[?]");
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	ImGui::SetItemTooltip("Conjunction of space separated predicates.\npredicate syntax: [-][filename=|tag=]<pattern>");
	ImGui::SameLine();
	ImGui::BeginDisabled(!file_list);
	ret |= ImGui::Button("search");
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("clear")) {
		m_query_input.clear();
		ret = true;
	}

	if (ret) { file_list->filter_by_query(m_query_input.as_view()); }
	return ret;
}

void FileBrowser::update_current_page() {
	if (!file_list) { return; }

	auto const page = file_list->get_current_page();

	auto number_width = 1;
	for (auto count = page.offset_from_start + int(page.entries.size()); count >= 10; count /= 10) { ++number_width; }

	for (auto const& [index, entry] : std::views::enumerate(page.entries)) {
		auto const& data = EntryData::read_from(*entry);
		auto const number = int(index) + page.offset_from_start + 1;
		ImGui::TextUnformatted(klib::FixedString{"{: >{}}.", number, number_width}.c_str());
		ImGui::SameLine();
		if (ImGui::Selectable(data.relative_path.c_str(), entry == &file_list->get_selected())) { file_list->select_entry(*entry); }
	}
}
} // namespace xtag::gui::ui::widget
