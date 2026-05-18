#pragma once
#include "app/file_list.hpp"
#include "ui/widget/im_input_text.hpp"
#include <array>

namespace xtag::gui::ui::widget {
class FileBrowser {
  public:
	static constexpr auto page_limits_v = std::array{10, 20, 50, 100, 200};
	static constexpr auto default_page_limit_v = page_limits_v[3];

	explicit FileBrowser(EntryList entry_list) : list(std::move(entry_list), default_page_limit_v) {}

	void update_pagination();
	[[nodiscard]] auto update_filter(std::string_view& out_query) -> bool;

	void update_number_width(FileList::Page const& current_page);
	void update_entry(int number, Entry const& entry, klib::CString path);

	FileList list;

  private:
	ImInputText m_query_input{};
	int m_number_width{};
};
} // namespace xtag::gui::ui::widget
