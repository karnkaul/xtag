#pragma once
#include "klib/base_types.hpp"
#include "klib/ptr.hpp"
#include "xtag/types.hpp"

namespace xtag::gui {
class FileBrowser : public klib::Pinned {
  public:
	explicit FileBrowser(EntryList list) { refresh(std::move(list)); }

	[[nodiscard]] auto get_view() const -> std::span<klib::Ptr<Entry const> const> { return m_view; }
	[[nodiscard]] auto get_selected() const -> klib::Ptr<Entry const> { return m_selected; }

	void refresh(EntryList list);

	auto select_entry(Entry const& subentry) -> bool;

	void apply_filter(std::string_view allowlist, std::string_view blocklist);

  private:
	[[nodiscard]] auto find_entry(fs::path const& path) const -> klib::Ptr<Entry const>;

	auto on_select(Entry const& entry) -> bool;

	EntryList m_list{};
	std::vector<klib::Ptr<Entry const>> m_view{};
	klib::Ptr<Entry const> m_selected{};
};
} // namespace xtag::gui
