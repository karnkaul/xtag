#pragma once
#include "app/file_list.hpp"
#include "ui/widget/im_input_text.hpp"
#include <array>
#include <optional>

namespace xtag::gui::ui::widget {
class FileBrowser {
  public:
	static constexpr auto page_limits_v = std::array{10, 20, 50, 100, 200};
	static constexpr auto default_page_limit_v = page_limits_v[3];

	void update_pagination();
	auto update_filter() -> bool;
	void update_current_page();

	std::optional<FileList> file_list{};

  private:
	ImInputText m_query_input{};
};
} // namespace xtag::gui::ui::widget
