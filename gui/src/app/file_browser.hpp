#pragma once
#include "klib/base_types.hpp"
#include "klib/ptr.hpp"
#include "klib/string/c_string.hpp"
#include "xtag/types.hpp"

namespace xtag::gui {
struct Directory {
	klib::Ptr<Entry const> entry{};
	klib::CString relative_path{};
};

class FileBrowser : public klib::Pinned {
  public:
	explicit FileBrowser(Entry root) { refresh(std::move(root)); }

	[[nodiscard]] auto get_root() const -> Entry const& { return m_root; }
	[[nodiscard]] auto get_pwd() const -> Directory;

	[[nodiscard]] auto get_selected() const -> klib::Ptr<Entry const> { return m_selected; }

	[[nodiscard]] auto has_parent() const -> bool;
	[[nodiscard]] auto get_parent() const -> klib::Ptr<Entry const>;

	void refresh(Entry root);

	auto select_subentry(Entry const& subentry) -> bool;
	void select_pwd();
	auto select_parent() -> bool;

	auto open_parent() -> bool;
	auto open_selected() -> bool;
	void open_root();

  private:
	[[nodiscard]] auto find_subentry(fs::path const& path) const -> klib::Ptr<Entry const>;

	auto on_pwd_changed() -> bool;
	auto on_select(Entry const& entry) -> bool;

	Entry m_root{};
	std::vector<klib::Ptr<Entry const>> m_stack{};
	klib::Ptr<Entry const> m_selected{};
	std::string m_pwd_rel_path{};
};
} // namespace xtag::gui
