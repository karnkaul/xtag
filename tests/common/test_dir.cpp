#include "common/test_dir.hpp"
#include <filesystem>
#include <fstream>

namespace xtag {
TestDir::TestDir(fs::path path) : m_path(std::move(path)) {
	if (fs::exists(m_path)) { fs::remove_all(m_path); }
	fs::create_directories(m_path);
}

TestDir::~TestDir() { fs::remove_all(m_path); }

auto TestDir::create_empty_file(fs::path const& subpath) const -> fs::path {
	if (subpath.empty()) { return {}; }
	auto file_path = m_path / subpath;
	[[maybe_unused]] auto file = std::ofstream{file_path};
	return file_path;
}

auto TestDir::create_directory(fs::path const& subpath) const -> fs::path {
	if (subpath.empty()) { return {}; }
	auto ret = m_path / subpath;
	if (fs::exists(ret)) { fs::remove_all(ret); }
	fs::create_directories(ret);
	return ret;
}
} // namespace xtag
