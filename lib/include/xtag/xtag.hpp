#pragma once
#include "xtag/result.hpp"
#include "xtag/tag_storage.hpp"
#include <filesystem>
#include <span>
#include <vector>

namespace xtag {
namespace fs = std::filesystem;

struct TagEntry {
	fs::path path{};
	std::vector<std::string_view> tags{};
};

auto get_tags_to(TagStorage& storage, std::vector<std::string_view>& out, fs::path const& path) -> Result<void>;
[[nodiscard]] auto get_tags(TagStorage& storage, fs::path const& path) -> Result<std::vector<std::string_view>>;

auto replace_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;
auto append_tags(TagStorage& storage, fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;

auto erase_tags(fs::path const& path) -> Result<void>;

void get_tagged_to(TagStorage& storage, std::vector<TagEntry>& out, fs::path const& directory, DirectoryParams const& params = {});
[[nodiscard]] auto get_tagged(TagStorage& storage, fs::path const& directory, DirectoryParams const& params = {}) -> std::vector<TagEntry>;
} // namespace xtag
