#pragma once
#include "klib/string/c_string.hpp"
#include "xtag/types.hpp"
#include <span>
#include <vector>

namespace xtag::gui {
struct EntryModel {
	klib::CString tree_filename{};
	klib::CString inspect_filename{};
	klib::CString short_tags{};
	std::span<std::string const> all_tags{};
};

struct EntryData {
	[[nodiscard]] static auto from(Entry const& entry) -> EntryData;

	[[nodiscard]] auto to_model() const -> EntryModel;

	std::string tree_filename{};
	std::string inspect_filename{};
	std::string short_tags{};
	std::vector<std::string> all_tags{};
};
} // namespace xtag::gui
