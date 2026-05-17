#pragma once
#include "ui/dispatch.hpp"
#include "ui/widget/file_browser.hpp"
#include "ui/widget/scan_data.hpp"
#include <optional>

namespace xtag::gui::ui {
class MainWindow : public Object {
  public:
	explicit MainWindow(IDispatch& dispatch) : m_dispatch(&dispatch) {}

	void update() final;

	void set_list(EntryList list);

	widget::ScanData scan_data{};

  private:
	void update_current_page();

	klib::Ptr<IDispatch> m_dispatch{};

	std::string m_root_directory{};
	std::optional<widget::FileBrowser> m_file_browser{};
};
} // namespace xtag::gui::ui
