#pragma once
#include "xtag/result.hpp"
#include "xtag/tag_storage.hpp"
#include <array>
#include <filesystem>
#include <vector>

namespace xtag {
namespace fs = std::filesystem;

struct TaggedEntry {
	fs::path path{};
	std::vector<std::string_view> tags{};
};

class Instance {
  public:
	struct Storage {
		TagStorage tags{};
	};

	[[nodiscard]] auto get_tags(fs::path const& path) -> Result<std::vector<std::string_view>>;

	auto replace_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;
	auto append_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;

	static auto erase_tags(fs::path const& path) -> Result<void>;

	[[nodiscard]] auto scan_tagged(fs::path const& directory, DirectoryParams const& params = {}) -> std::vector<TaggedEntry>;

	Storage storage{};

  private:
	[[nodiscard]] auto get_serialized(fs::path const& path) -> Result<std::string_view>;

	std::array<std::string, 2> m_buffers{};
};
} // namespace xtag
