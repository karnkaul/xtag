#include "klib/debug/assert.hpp"
#include "klib/string/c_string.hpp"
#include "xtag/result.hpp"
#include "xtag/xattr.hpp"
#include <cerrno>
#include <format>
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
	if (path.as_view().empty()) { return to_error(Error::Type::InvalidArgument, "xattr::get(): passed path is empty"); }
	if (name.as_view().empty()) { return to_error(Error::Type::InvalidArgument, "xattr::get(): passed name is empty"); }
	return {};
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
} // namespace xtag

auto xtag::format_error(Error::Type const type, std::string_view const message) -> std::string {
	return std::format("[{}] {}", Error::type_name_map.to_name(type), message);
}

auto xtag::to_error(Error::Type const type, std::string_view const message) -> std::unexpected<Error> {
	auto msg = format_error(type, message);
	return std::unexpected{Error{.type = type, .message = std::move(msg)}};
}
