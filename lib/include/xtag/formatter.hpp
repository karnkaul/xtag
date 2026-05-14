#pragma once
#include "xtag/types.hpp"

namespace xtag {
struct Formatter {
	void join_to(std::string& out, std::string_view item) const;
	[[nodiscard]] auto join(std::string_view item) const -> std::string;

	void truncate_to(std::string& out, std::span<std::string const> items, int max_count) const;
	[[nodiscard]] auto truncate(std::span<std::string const> items, int max_count) const -> std::string;

	[[nodiscard]] auto format(ScanTag const& tag) const -> std::string;
	void join_to(std::string& out, std::span<ScanTag const> tags) const;
	[[nodiscard]] auto join(std::span<ScanTag const> tags) const -> std::string;

	void format_to(std::string& out, Entry const& entry, fs::path const& root = {}) const;
	[[nodiscard]] auto format(Entry const& entry, fs::path const& root = {}) const -> std::string;

	[[nodiscard]] auto format_table(EntryList const& list) const -> std::string;

	std::string_view inherited_prefix{"*"};
	std::string_view delimiter{", "};
	std::string_view truncator{"..."};
};
} // namespace xtag
