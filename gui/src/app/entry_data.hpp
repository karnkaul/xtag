#pragma once
#include "klib/string/c_string.hpp"
#include "xtag/types.hpp"
#include <span>
#include <vector>

namespace xtag::gui {
struct EntryModel {
	klib::CString tree_uri{};
	klib::CString inspect_uri{};
	klib::CString short_tags{};
	std::span<std::string const> all_tags{};
};

struct EntryData {
	[[nodiscard]] static auto from(Entry const& entry, fs::path const& root) -> EntryData;

	[[nodiscard]] auto to_model() const -> EntryModel;

	std::string tree_uri{};
	std::string inspect_uri{};
	std::string short_tags{};
	std::vector<std::string> all_tags{};
};
} // namespace xtag::gui
