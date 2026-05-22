#include "detail/util.hpp"
#include "xtag/xattr.hpp"
#include <fstream>

namespace xtag {
namespace {
[[nodiscard]] auto get_ads_path(klib::CString const in, klib::CString const name) -> fs::path { return std::format("{}:{}", in.as_view(), name.as_view()); }

auto file_to(std::string& out, fs::path const& path) -> bool {
	auto file = std::ifstream{path, std::ios::binary | std::ios::ate};
	if (!file) { return false; }

	auto const size = file.tellg();
	file.seekg(0, std::ios::beg);

	out.resize(std::size_t(size));
	file.read(out.data(), size);
	return true;
}

auto to_file(fs::path const& path, std::string_view text) -> bool {
	auto file = std::ofstream{path, std::ios::binary};
	if (!file) { return false; }

	file.write(text.data(), std::streamsize(text.size()));
	return file.good();
}
} // namespace

auto xattr::get_to(std::string& buffer, klib::CString const path, klib::CString const name) -> Result<std::string_view> {
	auto result = detail::validate_inputs(path, name);
	if (!result) { return std::unexpected{std::move(result.error())}; }

	auto const _ = detail::ScopedErrno{};
	auto err = std::error_code{};
	if (!fs::exists(path.as_view(), err)) { return to_error(Error::Type::InvalidArgument, std::format("Nonexistent path: '{}'", path.as_view())); }

	auto const ads_path = get_ads_path(path, name);
	if (!file_to(buffer, ads_path)) { return to_error(Error::Type::NoData, std::format("Failed to read ADS: '{}'", ads_path.generic_string())); }

	return buffer;
}

auto xattr::get(klib::CString const path, klib::CString const name) -> Result<std::string> {
	auto ret = std::string{};
	auto result = get_to(ret, path, name);
	if (!result) { return std::unexpected{std::move(result.error())}; }
	return ret;
}

auto xattr::set(klib::CString const path, klib::CString const name, klib::CString const value) -> Result<void> {
	return detail::validate_inputs(path, name).and_then([&] -> Result<void> {
		auto const ads_path = get_ads_path(path, name);
		auto const _ = detail::ScopedErrno{};
		if (!to_file(ads_path, value.as_view())) { return to_error(Error::Type::IOError, std::format("Failed to write ADS: '{}'", ads_path.generic_string())); }
		return {};
	});
}

auto xattr::remove(klib::CString const path, klib::CString const name) -> Result<void> {
	return detail::validate_inputs(path, name).and_then([&] -> Result<void> {
		auto const ads_path = get_ads_path(path, name);
		auto const _ = detail::ScopedErrno{};
		auto err = std::error_code{};
		if (!fs::exists(ads_path, err)) { return {}; }
		if (!fs::remove(ads_path, err)) { return to_error(Error::Type::IOError, std::format("Failed to delete ADS: '{}'", ads_path.generic_string())); }
		return {};
	});
}
} // namespace xtag
