#pragma once
#include "ui/dispatch.hpp"
#include "ui/widget/file_browser.hpp"
#include "ui/widget/scan_data.hpp"
#include "ui/widget/tag_editor.hpp"
#include <optional>

namespace xtag::gui::ui {
class MainWindow : public Object {
  public:
	explicit MainWindow(IDispatch& dispatch) : m_dispatch(&dispatch) {}

	void update() final;

	void set_list(EntryList list);
	void set_filter(std::string_view query);

	widget::ScanData scan_data{};

  private:
	void update_current_page();

	auto should_replace_tags() -> bool;

	klib::Ptr<IDispatch> m_dispatch{};

	std::string m_root_directory{};
	std::optional<widget::FileBrowser> m_file_browser{};
	widget::TagEditor m_tag_editor{};
	std::vector<std::string_view> m_tag_replacement{};
};
} // namespace xtag::gui::ui
