#include "detail/util.hpp"
#include "klib/cli/text_table.hpp"
#include "klib/debug/assert.hpp"
#include "xtag/format.hpp"
#include "xtag/instance.hpp"
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
constexpr auto tag_delimiter_v = ',';

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

[[nodiscard]] auto to_entry_type(fs::path const& path) -> EntryType {
	if (fs::is_directory(path)) { return EntryType::Directory; }
	if (fs::is_regular_file(path)) { return EntryType::File; }
	return EntryType::None;
}

[[nodiscard]] auto repoint_through(StringSet& out, std::string_view const tag) -> std::string_view {
	auto it = out.find(tag);
	if (it == out.end()) { it = out.insert(std::string{tag}).first; }
	KLIB_ASSERT(it != out.end());
	return *it;
}

[[nodiscard]] auto validate(klib::CString const path, klib::CString const name) -> Result<void> {
	if (path.as_view().empty()) { return to_error(Error::Type::InvalidArgument, "passed path is empty"); }
	if (name.as_view().empty()) { return to_error(Error::Type::InvalidArgument, "passed name is empty"); }
	return {};
}

[[nodiscard]] constexpr auto passes_filter(std::span<ScanTag const> scan_tags, std::span<std::string_view const> filter) -> bool {
	if (filter.empty()) { return !scan_tags.empty(); }
	auto const pred = [filter](ScanTag const& scan_tag) { return std::ranges::find(filter, scan_tag.value) != filter.end(); };
	return std::ranges::any_of(scan_tags, pred);
}
} // namespace

auto xattr::get_to(std::string& buffer, klib::CString const path, klib::CString const name) -> Result<std::string_view> {
	auto result = validate(path, name);
	if (!result) { return std::unexpected{std::move(result.error())}; }

	auto const _ = ScopedErrno{};
	if (buffer.empty()) {
		auto const buffer_size = ::getxattr(path.c_str(), name.c_str(), nullptr, 0);
		if (buffer_size == -1) { return from_errno(path.as_view(), name.as_view()); }
		buffer.resize(std::size_t(buffer_size));
	}

	auto const size = ::getxattr(path.c_str(), name.c_str(), buffer.data(), buffer.size());
	if (size == -1) { return from_errno(path.as_view(), name.as_view()); }

	KLIB_ASSERT(size <= ssize_t(buffer.size()));
	return std::string_view{buffer.data(), std::size_t(size)};
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

void detail::serialize_tags_to(std::string& out, std::span<std::string_view const> tags) {
	for (auto const& tag : tags) {
		if (!out.empty()) { out.push_back(tag_delimiter_v); }
		out.append(tag);
	}
}

void detail::deserialize_tags(StringSet& out_set, std::string_view const serialized, OnTagDeserialized per_tag) {
	for (auto const it : std::views::split(serialized, tag_delimiter_v)) {
		auto const tag = std::string_view{it};
		per_tag(repoint_through(out_set, tag));
	}
}

void detail::join_to(std::string& out, std::string_view const item, std::string_view const delimiter) {
	if (item.empty()) { return; }
	if (!out.empty()) { out.append(delimiter); }
	out.append(item);
}

class Instance::Scanner {
  public:
	explicit Scanner(Instance& self, ScanInfo const& info) : m_self(self), m_info(info) {}

	[[nodiscard]] auto operator()(fs::path const& root) -> std::vector<Entry> {
		if (fs::is_regular_file(root)) {
			on_entry(root, {}, EntryType::File);
		} else if (fs::is_directory(root)) {
			scan_directory(root, {}, 0);
		}
		return std::move(m_ret);
	}

  private:
	void scan_directory(fs::path const& path, std::span<ScanTag const> inherited, int const depth) {
		if ((m_info.filter.entry_type & EntryType::Directory) != EntryType::Directory) { return; }

		auto const scan_tags = on_entry(path, inherited, EntryType::Directory);

		for (auto const& it : fs::directory_iterator{path}) {
			if (it.is_directory()) {
				if (depth < m_info.depth) { scan_directory(it.path(), scan_tags, depth + 1); }
				continue;
			}

			if (it.is_regular_file() && (m_info.filter.entry_type & EntryType::File) == EntryType::File) { on_entry(path, scan_tags, EntryType::File); }
		}
	}

	auto on_entry(fs::path const& path, std::span<ScanTag const> inherited, EntryType const type) -> std::vector<ScanTag> {
		auto scan_tags = std::vector<ScanTag>{};
		if (auto result = m_self.get_tags(path)) { scan_tags = std::move(result->scan_tags); }
		for (auto const& scan_tag : inherited) { scan_tags.push_back(ScanTag{.value = scan_tag.value, .type = TagType::Inherited}); }
		if (scan_tags.empty()) { return {}; }
		if (!passes_filter(scan_tags, m_info.filter.tags)) { return scan_tags; }

		m_ret.push_back(Entry{.path = fs::canonical(path), .scan_tags = std::move(scan_tags), .type = type});
		return m_ret.back().scan_tags;
	}

	Instance& m_self;
	ScanInfo const& m_info;

	std::vector<Entry> m_ret{};
};

auto Instance::get_tags(fs::path const& path) -> Result<Entry> {
	auto& serialized = wipe_buffer();
	return get_serialized_to(serialized, path).transform([&] {
		auto ret = Entry{.path = fs::canonical(path)};
		ret.type = to_entry_type(ret.path);
		auto const per_tag = [&](std::string_view const tag) { ret.scan_tags.push_back(ScanTag{.value = tag, .type = TagType::Primary}); };
		detail::deserialize_tags(m_tag_storage, serialized, per_tag);
		return ret;
	});
}

auto Instance::replace_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void> {
	if (tags.empty()) { return erase_tags(path); }
	auto& serialized = wipe_buffer();
	detail::serialize_tags_to(serialized, tags);
	return xattr::set(path.generic_string().c_str(), get_attribute_name(), serialized);
}

auto Instance::append_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void> {
	auto& serialized = wipe_buffer();
	return get_serialized_to(serialized, path).and_then([&] {
		detail::serialize_tags_to(serialized, tags);
		return xattr::set(path.generic_string().c_str(), get_attribute_name(), serialized);
	});
}

auto Instance::erase_tags(fs::path const& path) const -> Result<void> {
	auto const str = path.generic_string();
	return xattr::remove(str.c_str(), get_attribute_name());
}

auto Instance::scan_tagged(fs::path const& directory, ScanInfo const& info) -> std::vector<Entry> { return Scanner{*this, info}(directory); }

auto Instance::get_attribute_name() const -> klib::CString {
	if (custom_attribute_name.empty()) { return default_attribute_name_v; }
	return custom_attribute_name;
}

auto Instance::wipe_buffer() -> std::string& {
	m_buffer.clear();
	return m_buffer;
}

auto Instance::get_serialized_to(std::string& out, fs::path const& path) const -> Result<void> {
	auto result = xattr::get_to(out, path.generic_string().c_str(), get_attribute_name());
	if (!result) {
		switch (result.error().type) {
		case Error::Type::NoData: return {};
		default: return std::unexpected{std::move(result.error())};
		}
	}
	return {};
}
} // namespace xtag

auto xtag::format_error(Error::Type const type, std::string_view const message) -> std::string {
	return std::format("[{}] {}", Error::type_name_map.to_name(type), message);
}

auto xtag::to_error(Error::Type const type, std::string_view const message) -> std::unexpected<Error> {
	auto msg = format_error(type, message);
	return std::unexpected{Error{.type = type, .message = std::move(msg)}};
}

auto xtag::format_table(std::span<Entry const> entries, FormatParams params) -> std::string {
	if (entries.empty()) { return {}; }

	if (!params.transform_path) {
		params.transform_path = [](fs::path const& path) { return path; };
	}

	auto table = klib::TextTable::Builder{}
					 .add_column("#", klib::TextTable::Align::Right)
					 .add_column(std::string{params.path_header})
					 .add_column(std::string{params.tags_header})
					 .build();
	for (auto const [index, entry] : std::views::enumerate(entries)) {
		std::string_view const type_str = entry.type == EntryType::Directory ? "d" : "f";

		auto row = std::vector<std::string>{};
		row.push_back(std::format("{}", index + 1));

		auto& path_str = row.emplace_back();
		path_str = std::format("[{}] {}", type_str, params.transform_path(entry.path).generic_string());

		auto& tags_str = row.emplace_back();
		for (auto const& tag : entry.scan_tags) {
			if (tag.type == TagType::Inherited) {
				detail::join_to(tags_str, std::format("*{}", tag.value), ", ");
			} else {
				detail::join_to(tags_str, tag.value, ", ");
			}
		}

		table.push_row(std::move(row));
	}

	return table.serialize();
}
