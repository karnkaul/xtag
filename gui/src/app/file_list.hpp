#pragma once
#include "klib/base_types.hpp"
#include "klib/ptr.hpp"
#include "xtag/types.hpp"
#include <unordered_map>

namespace xtag::gui {
class FileList : public klib::Pinned {
  public:
	struct Page {
		std::span<klib::Ptr<Entry> const> entries{};
		int offset_from_start{};
	};

	static constexpr auto min_page_limit_v{10};
	static constexpr auto max_page_limit_v{1000};

	explicit FileList(EntryList list, int page_limit = max_page_limit_v);

	[[nodiscard]] auto get_root() const -> fs::path const& { return m_list.path; }
	[[nodiscard]] auto get_filtered() const -> std::span<klib::Ptr<Entry> const> { return m_filtered; }
	[[nodiscard]] auto get_selected() const -> Entry& { return *m_selected.entry; }
	[[nodiscard]] auto get_current_page() const -> Page;
	[[nodiscard]] auto get_page_number() const -> int { return m_page_number; }
	[[nodiscard]] auto get_page_count() const -> int { return m_page_count; }
	[[nodiscard]] auto get_page_limit() const -> int { return m_page_limit; }

	void refresh(EntryList list);
	void repaginate(int page_limit);

	auto select_entry(Entry const& subentry) -> bool;
	auto set_page_number(int page_number) -> bool;

	template <typename PredT>
	void apply_filter(PredT should_include) {
		m_filtered.clear();
		for (auto& entry : m_list.entries) {
			if (!should_include(entry)) { continue; }
			m_filtered.emplace_back(&entry);
		}
		repaginate(m_page_limit);
	}

	void clear_filter();

  private:
	struct EntryView {
		klib::Ptr<Entry> entry{};
		int index{-1};
	};

	void clear_pointers(std::size_t reserve = 0);
	[[nodiscard]] auto find_entry(fs::path const& path) const -> EntryView;

	EntryList m_list{};
	std::unordered_map<fs::path, EntryView> m_path_map{};
	std::vector<klib::Ptr<Entry>> m_filtered{};

	EntryView m_selected{};
	int m_page_limit{max_page_limit_v};
	int m_page_number{};
	int m_page_count{};
};
} // namespace xtag::gui
