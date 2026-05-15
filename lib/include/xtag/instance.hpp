#pragma once
#include "klib/string/c_string.hpp"
#include "xtag/result.hpp"
#include "xtag/string_set.hpp"

namespace xtag {
struct ScanFilter {
	std::span<std::string_view const> tags{};
	TagType tag_type{TagType::Primary | TagType::Inherited};
	bool include_files{true};
};

struct ScanInfo {
	using Filter = ScanFilter;

	Filter filter{};
	int depth{5};
};

class Instance {
  public:
	static constexpr auto default_attribute_name_v = klib::CString{"user.xdg.tags"};

	explicit Instance(std::string custom_attribute_name = {}) : custom_attribute_name(std::move(custom_attribute_name)) {}

	[[nodiscard]] auto get_tags(fs::path const& path) -> Result<EntryOld>;

	auto replace_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;
	auto append_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;

	auto erase_tags(fs::path const& path) const -> Result<void>;

	[[nodiscard]] auto scan_directory_old(fs::path const& directory, ScanInfo const& info = {}) -> Result<EntryOld>;
	[[nodiscard]] auto scan_directory(fs::path const& directory, ScanInfo const& info = {}) -> Result<EntryList>;

	[[nodiscard]] auto get_attribute_name() const -> klib::CString;
	[[nodiscard]] auto get_tag_storage() const -> StringSet const& { return m_tag_storage; }
	void clear_tag_storage() { m_tag_storage.clear(); }

	std::string custom_attribute_name{};

  private:
	[[nodiscard]] auto wipe_buffer() -> std::string&;

	[[nodiscard]] auto get_serialized_to(std::string& out, fs::path const& path) const -> Result<void>;

	StringSet m_tag_storage{};
	std::string m_buffer{};
};
} // namespace xtag
