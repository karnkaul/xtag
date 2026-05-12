#pragma once
#include "app/entry_data.hpp"
#include "app/file_browser.hpp"
#include "app/object.hpp"
#include "app/scan_data.hpp"
#include "klib/string/str_buf.hpp"
#include <optional>

namespace xtag::gui {
class MainWindow : public Object {
  public:
	void update() final;

	void refresh_root_directory(Entry directory);

	ScanData scan_data{};

  private:
	void update_controls();
	void update_navigation();
	void update_inspector(EntryModel const& entry);
	void update_browser();

	std::optional<FileBrowser> m_file_browser{};
	std::string m_root_directory{};
	klib::StrBuf<127> m_filename_filter{};
	bool m_open_tag_viewer{};
};
} // namespace xtag::gui
