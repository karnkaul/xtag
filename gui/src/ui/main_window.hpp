#pragma once
#include "app/file_browser.hpp"
#include "klib/string/str_buf.hpp"
#include "ui/controller.hpp"
#include "ui/scan_data.hpp"
#include <optional>

namespace xtag::gui::ui {
class MainWindow : public Object {
  public:
	explicit MainWindow(IController& controller) : m_controller(&controller) {}

	void update() final;

	void set_list(EntryList list);

	ScanData scan_data{};

  private:
	struct Filter {
		klib::StrBuf<127> allow{};
		klib::StrBuf<127> block{};
	};

	void update_scan_data();
	void update_filters();
	void update_pagination();
	void update_inspector(Entry const& selected);
	void update_list(Entry const& selected);

	klib::Ptr<IController> m_controller{};

	std::string m_root_directory{};
	std::optional<FileBrowser> m_file_browser{};

	Filter m_filter{};
	bool m_open_tag_viewer{};
};
} // namespace xtag::gui::ui
