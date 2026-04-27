#include "xtag/xtag.hpp"
#include "detail/util.hpp"
#include "klib/debug/assert.hpp"
#include "klib/string/c_string.hpp"
#include "xtag/result.hpp"
#include "xtag/tag_storage.hpp"
#include "xtag/types.hpp"
#include "xtag/xattr.hpp"
#include <cerrno>
#include <filesystem>
#include <format>
#include <ranges>
#include <string_view>

#if !defined(__linux__)
#error "unsupported platform"
#endif

#include <sys/xattr.h>

namespace xtag {
namespace {
class ScopedErrno {
  public:
	ScopedErrno(ScopedErrno const&) = delete;
	ScopedErrno(ScopedErrno&&) = delete;
	ScopedErrno& operator=(ScopedErrno const&) = delete;
	ScopedErrno& operator=(ScopedErrno&&) = delete;

	ScopedErrno() = default;
	~ScopedErrno() { errno = 0; }
};

[[nodiscard]] auto from_errno(std::string_view const path, std::string_view const attr_name) -> std::unexpected<Error> {
	auto const value = errno;
	KLIB_ASSERT(value != 0);
	switch (value) {
	default: return to_error(Error::Type::Unknown, "unknown error");

	// stat
	case EPERM:
	case EACCES: return to_error(Error::Type::AccessDenied, std::format("access denied: '{}'", path));
	case ENOTDIR:
	case ENOENT: return to_error(Error::Type::InvalidArgument, std::format("invalid path: '{}'", path));
	case ENAMETOOLONG: return to_error(Error::Type::PathTooLong, std::format("path too long: '{}'", path));

	// xattr
	case ENOTSUP: return to_error(Error::Type::NotSupported, "xattr not supported / invalid namespace prefix");
	case ENODATA: return to_error(Error::Type::NoData, std::format("attribute not present: '{}'", attr_name));
	case ERANGE:
	case E2BIG: return to_error(Error::Type::TooBig, std::format("attribute name/value too large: '{}'", attr_name));
	}
}

[[nodiscard]] auto validate(klib::CString const path, klib::CString const name) -> Result<void> {
	if (path.as_view().empty()) { return to_error(Error::Type::InvalidArgument, "passed path is empty"); }
	if (name.as_view().empty()) { return to_error(Error::Type::InvalidArgument, "passed name is empty"); }
	return {};
}

constexpr auto tag_name_v = klib::CString{"user.xtag"};

template <typename Func>
void iterate_directory(fs::path const& directory, DirectoryParams const& params, Func const& per_entry, int const depth = 0) {
	if (!fs::is_directory(directory)) { return; }
	for (auto const& it : fs::directory_iterator{directory}) {
		if (it.is_regular_file()) {
			if ((params.filter & Filter::Directory) == Filter::Directory) { per_entry(it.path()); }
			continue;
		}

		if (!it.is_directory()) { continue; }
		if (depth >= params.depth) { continue; }
		iterate_directory(it.path(), params, per_entry, depth + 1);
	}
}
} // namespace

auto xattr::get_to(std::string& buffer, klib::CString const path, klib::CString const name) -> Result<std::string_view> {
	if (buffer.empty()) { return to_error(Error::Type::InvalidArgument, "xattr::get(): passed buffer is empty"); }

	return validate(path, name).and_then([&] -> Result<std::string_view> {
		auto const _ = ScopedErrno{};
		auto const result = ::getxattr(path.c_str(), name.c_str(), buffer.data(), buffer.size());
		if (result == -1) { return from_errno(path.as_view(), name.as_view()); }

		KLIB_ASSERT(result <= ssize_t(buffer.size()));
		return std::string_view{buffer.data(), std::size_t(result)};
	});
}

auto xattr::get(klib::CString const path, klib::CString const name) -> Result<std::string> {
	auto const _ = ScopedErrno{};
	auto const buffer_size = ::getxattr(path.c_str(), name.c_str(), nullptr, 0);
	if (buffer_size == -1) { return from_errno(path.as_view(), name.as_view()); }

	auto buffer = std::string{};
	buffer.resize(std::size_t(buffer_size));
	return get_to(buffer, path, name).transform([](std::string_view const ret) { return std::string{ret}; });
}

auto xattr::set(klib::CString const path, klib::CString const name, klib::CString const value) -> Result<void> {
	return validate(path, name).and_then([&] -> Result<void> {
		auto const _ = ScopedErrno{};
		auto const result = ::setxattr(path.c_str(), name.c_str(), value.c_str(), value.as_view().size(), 0);
		if (result == -1) { return from_errno(path.as_view(), name.as_view()); }
		return {};
	});
}

auto xattr::remove(klib::CString const path, klib::CString const name) -> Result<void> {
	return validate(path, name).and_then([&] -> Result<void> {
		auto const _ = ScopedErrno{};
		auto const result = ::removexattr(path.c_str(), name.c_str());
		if (result == -1) { return from_errno(path.as_view(), name.as_view()); }
		return {};
	});
}

void detail::deserialize_tags_to(std::vector<std::string>& out, std::string_view const serialized) {
	for (auto const tag : std::views::split(serialized, '|')) { out.emplace_back(std::string_view{tag}); }
}

auto detail::deserialize_tags(std::string_view const serialized) -> std::vector<std::string> {
	auto ret = std::vector<std::string>{};
	deserialize_tags_to(ret, serialized);
	return ret;
}

void detail::serialize_tags_to(std::string& out, std::span<std::string_view const> tags) {
	for (auto const& tag : tags) {
		if (!out.empty()) { out.push_back('|'); }
		out.append(tag);
	}
}

auto detail::serialize_tags(std::span<std::string_view const> tags) -> std::string {
	auto ret = std::string{};
	serialize_tags_to(ret, tags);
	return ret;
}

void detail::combine_tags_to(std::vector<std::string_view>& out, std::span<std::string_view const> a, std::span<std::string_view const> b) {
	out.reserve(out.size() + a.size() + b.size());
	for (std::string_view const str : a) { out.push_back(str); }
	for (std::string_view const str : b) { out.push_back(str); }
}

auto detail::combine_tags(std::span<std::string_view const> a, std::span<std::string_view const> b) -> std::vector<std::string_view> {
	auto ret = std::vector<std::string_view>{};
	combine_tags_to(ret, a, b);
	return ret;
}

auto TagStorage::insert_tag(std::string tag) -> std::string_view {
	auto it = m_tags.find(tag);
	if (it == m_tags.end()) { it = m_tags.insert(std::move(tag)).first; }
	return *it;
}
} // namespace xtag

auto xtag::format_error(Error::Type const type, std::string_view const message) -> std::string {
	return std::format("[{}] {}", Error::type_name_map.to_name(type), message);
}

auto xtag::to_error(Error::Type const type, std::string_view const message) -> std::unexpected<Error> {
	auto msg = format_error(type, message);
	return std::unexpected{Error{.type = type, .message = std::move(msg)}};
}

auto xtag::get_tags_to(TagStorage& storage, std::vector<std::string_view>& out, fs::path const& path) -> Result<void> {
	auto result = xattr::get(path.generic_string().c_str(), tag_name_v);
	if (!result) {
		switch (result.error().type) {
		case Error::Type::NoData: return {};
		default: return std::unexpected{std::move(result.error())};
		}
	}

	for (auto const it : std::views::split(*result, '|')) {
		auto const tag = std::string_view{it};
		out.push_back(storage.insert_tag(std::string{tag}));
	}
	return {};
}

auto xtag::get_tags(TagStorage& storage, fs::path const& path) -> Result<std::vector<std::string_view>> {
	auto ret = std::vector<std::string_view>{};
	return get_tags_to(storage, ret, path).transform([&] { return std::move(ret); });
}

auto xtag::replace_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void> {
	auto const serialized = detail::serialize_tags(tags);
	return xattr::set(path.generic_string().c_str(), tag_name_v, serialized);
}

auto xtag::append_tags(TagStorage& storage, fs::path const& path, std::span<std::string_view const> tags) -> Result<void> {
	return get_tags(storage, path).and_then([&](std::span<std::string_view const> current) {
		auto const combined = detail::combine_tags(current, tags);
		return replace_tags(path, combined);
	});
}

auto xtag::erase_tags(fs::path const& path) -> Result<void> {
	auto const str = path.generic_string();
	return xattr::remove(str.c_str(), tag_name_v);
}

void xtag::get_tagged_to(TagStorage& storage, std::vector<TagEntry>& out, fs::path const& directory, DirectoryParams const& params) {
	auto const per_entry = [&](fs::path path) {
		auto tags = get_tags(storage, path);
		if (!tags) { return; }
		out.push_back(TagEntry{.path = std::move(path), .tags = std::move(*tags)});
	};
	iterate_directory(directory, params, per_entry);
}

auto xtag::get_tagged(TagStorage& storage, fs::path const& directory, DirectoryParams const& params) -> std::vector<TagEntry> {
	auto ret = std::vector<TagEntry>{};
	get_tagged_to(storage, ret, directory, params);
	return ret;
}
