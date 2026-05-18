#pragma once
#include "ui/dispatch.hpp"
#include "ui/widget/file_browser.hpp"
#include "ui/widget/scan_data.hpp"
#include "ui/widget/tag_editor.hpp"

namespace xtag::gui::ui {
class MainWindow : public Object {
  public:
	explicit MainWindow(IDispatch& dispatch) : m_dispatch(&dispatch) {}

	void update() final;

	void set_list(std::shared_ptr<EntryList const> list);
	void set_filter(std::string_view query);

	widget::ScanData scan_data{};

  private:
	enum class Action : std::int8_t { None, RefreshRoot, ReplaceTags };

	void update_header();
	void update_table();
	void update_controls();
	void update_current_page();
	void update_tag_editor();

	void set_tag_replacement();

	klib::Ptr<IDispatch> m_dispatch{};

	std::string m_root_directory{};
	widget::FileBrowser m_file_browser{};
	widget::TagEditor m_tag_editor{};

	std::vector<std::string_view> m_tag_replacement{};
	Action m_action{};
};
} // namespace xtag::gui::ui
