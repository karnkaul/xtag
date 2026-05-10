#pragma once
#include "klib/string/c_string.hpp"
#include "xtag/result.hpp"
#include "xtag/string_set.hpp"
#include <vector>

namespace xtag {
struct ScanFilter {
	std::span<std::string_view const> tags{};
	EntryType entry_type{EntryType::Directory | EntryType::File};
	TagType tag_type{TagType::Primary | TagType::Inherited};
};

struct ScanInfo {
	using Filter = ScanFilter;

	Filter filter{};
	int depth{10};
};

class Instance {
  public:
	static constexpr auto default_attribute_name_v = klib::CString{"user.xdg.tags"};

	explicit Instance(std::string custom_attribute_name = {}) : custom_attribute_name(std::move(custom_attribute_name)) {}

	[[nodiscard]] auto get_tags(fs::path const& path) -> Result<Entry>;

	auto replace_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;
	auto append_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;

	auto erase_tags(fs::path const& path) const -> Result<void>;

	[[nodiscard]] auto scan_tagged(fs::path const& directory, ScanInfo const& info = {}) -> std::vector<Entry>;

	[[nodiscard]] auto get_attribute_name() const -> klib::CString;
	[[nodiscard]] auto get_tag_storage() const -> StringSet const& { return m_tag_storage; }
	void clear_tag_storage() { m_tag_storage.clear(); }

	std::string custom_attribute_name{};

  private:
	class Scanner;

	[[nodiscard]] auto wipe_buffer() -> std::string&;

	[[nodiscard]] auto get_serialized_to(std::string& out, fs::path const& path) const -> Result<void>;

	StringSet m_tag_storage{};
	std::string m_buffer{};
};
} // namespace xtag
