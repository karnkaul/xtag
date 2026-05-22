#pragma once
#include "ui/action.hpp"
#include "ui/widget/entry_browser.hpp"
#include "ui/widget/scan_data.hpp"
#include "ui/widget/tag_editor.hpp"
#include "xtag/string_set.hpp"

namespace xtag::gui::ui {
class MainWindow {
  public:
	explicit MainWindow(StringSet& tag_storage);

	auto update() -> Action;

	void set_list(std::shared_ptr<EntryDataList const> list);

	[[nodiscard]] auto get_selected() const -> klib::Ptr<Entry const>;
	[[nodiscard]] auto get_replacement_tags() const -> std::span<std::string_view const>;

	widget::ScanData scan_data{};

  private:
	void update_header();
	void update_table();
	void update_controls();
	void update_current_page();
	void update_tag_editor();

	widget::EntryBrowser m_entry_browser{};
	widget::TagEditor m_tag_editor;
	bool m_open_tag_editor{};

	Action m_action{};
};
} // namespace xtag::gui::ui
