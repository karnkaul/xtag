#pragma once
#include "klib/base_types.hpp"
#include "klib/ptr.hpp"
#include "ui/entry_data.hpp"
#include <unordered_map>

namespace xtag::gui::ui {
class EntryBook : public klib::Pinned {
  public:
	struct Page {
		std::span<klib::Ptr<EntryData const> const> entries{};
		int offset_from_start{};
	};

	static constexpr auto min_page_limit_v{10};
	static constexpr auto max_page_limit_v{1000};

	explicit EntryBook(std::shared_ptr<EntryDataList const> list, int page_limit = max_page_limit_v);

	[[nodiscard]] auto get_root() const -> std::string_view { return m_list->path; }
	[[nodiscard]] auto get_filtered() const -> std::span<klib::Ptr<EntryData const> const> { return m_filtered.empty() ? m_all : m_filtered; }
	[[nodiscard]] auto get_selected() const -> EntryData const& { return *m_selected.entry_data; }
	[[nodiscard]] auto get_current_page() const -> Page;
	[[nodiscard]] auto get_page_number() const -> int { return m_page_number; }
	[[nodiscard]] auto get_page_count() const -> int { return m_page_count; }
	[[nodiscard]] auto get_page_limit() const -> int { return m_page_limit; }

	void refresh(std::shared_ptr<EntryDataList const> list, int new_page_limit = -1);
	void repaginate(int page_limit);

	auto select_entry(EntryData const& entry_data) -> bool;
	auto set_page_number(int page_number) -> bool;

	void filter_by_query(std::string_view query);
	void clear_filter();

  private:
	struct EntryView {
		klib::Ptr<EntryData const> entry_data{};
		int index{-1};
	};

	void clear_pointers(std::size_t reserve = 0);
	[[nodiscard]] auto find_entry(fs::path const& path) const -> EntryView;
	[[nodiscard]] auto page_number_for(EntryData const& entry) const -> int;

	std::shared_ptr<EntryDataList const> m_list{};
	std::unordered_map<fs::path, EntryView> m_path_map{};
	std::vector<klib::Ptr<EntryData const>> m_all{};
	std::vector<klib::Ptr<EntryData const>> m_filtered{};

	EntryView m_selected{};
	int m_page_limit{max_page_limit_v};
	int m_page_number{};
	int m_page_count{};
};
} // namespace xtag::gui::ui
