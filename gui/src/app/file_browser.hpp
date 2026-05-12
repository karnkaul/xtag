#pragma once
#include "klib/ptr.hpp"
#include "klib/string/c_string.hpp"
#include "xtag/types.hpp"

namespace xtag::gui {
struct Directory {
	klib::Ptr<Entry const> entry{};
	klib::CString relative_path{};
};

class FileBrowser {
  public:
	explicit FileBrowser(Entry root) { refresh(std::move(root)); }

	[[nodiscard]] auto get_root() const -> Entry const& { return m_root; }
	[[nodiscard]] auto get_pwd() const -> Directory;

	void refresh(Entry root);

	[[nodiscard]] auto get_selected() const -> klib::Ptr<Entry const> { return m_selected; }
	auto set_selected(Entry const& entry) -> bool;

	[[nodiscard]] auto can_navigate_up() const -> bool;
	void navigate_up();

	auto navigate_to_selected() -> bool;

	void navigate_to_root();

  private:
	void refresh_data();

	Entry m_root{};
	std::vector<klib::Ptr<Entry const>> m_stack{};
	klib::Ptr<Entry const> m_selected{};
	std::string m_pwd_rel_path{};
};
} // namespace xtag::gui
