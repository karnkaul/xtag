#include "detail/util.hpp"
#include "klib/debug/assert.hpp"
#include "xtag/formatter.hpp"
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

[[nodiscard]] constexpr auto should_include(std::span<ScanTag const> scan_tags, ScanFilter const& filter) -> bool {
	if (filter.tags.empty()) { return (filter.tag_type & TagType::Untagged) == TagType::Untagged || !scan_tags.empty(); }
	auto const pred = [filter](ScanTag const& scan_tag) { return std::ranges::find(filter.tags, scan_tag.value) != filter.tags.end(); };
	return std::ranges::any_of(scan_tags, pred);
}

[[nodiscard]] auto to_entry_type(fs::path const& path) -> EntryType {
	if (fs::is_directory(path)) { return EntryType::Directory; }
	if (fs::is_regular_file(path)) { return EntryType::File; }
	return EntryType::None;
}

class Scanner {
  public:
	explicit Scanner(Instance& self, ScanInfo const& info) : m_self(self), m_info(info) {}

	[[nodiscard]] auto operator()(fs::path const& root) -> Result<Entry> {
		if (!fs::is_directory(root)) { return to_error(Error::Type::InvalidArgument, std::format("not a directory: '{}'", root.generic_string())); }
		return scan_directory(root, {}, 0);
	}

  private:
	[[nodiscard]] auto scan_directory(fs::path const& path, std::span<ScanTag const> parent_tags, int const depth) -> Entry {
		auto ret = Entry{.type = EntryType::Directory};
		ret.path = fs::canonical(path);
		ret.tags = get_combined_tags(path, parent_tags);

		for (auto const& it : fs::directory_iterator{path}) {
			if (it.is_directory()) {
				scan_subdirectory(ret, it.path(), depth + 1);
			} else if (it.is_regular_file()) {
				scan_file(ret, it.path());
			}
		}

		return ret;
	}

	[[nodiscard]] auto get_combined_tags(fs::path const& path, std::span<ScanTag const> parent_tags) const -> std::vector<ScanTag> {
		auto ret = std::vector<ScanTag>{};
		if (auto result = m_self.get_tags(path)) { ret = std::move(result->tags); }
		for (auto const& scan_tag : parent_tags) { ret.push_back(ScanTag{.value = scan_tag.value, .type = TagType::Inherited}); }
		return ret;
	}

	void scan_subdirectory(Entry& parent, fs::path const& path, int const depth) {
		if (depth > m_info.depth) { return; }
		auto subdir = scan_directory(path, parent.tags, depth);
		if (!should_include(subdir.tags, m_info.filter)) { return; }
		parent.subentries.push_back(std::move(subdir));
	}

	void scan_file(Entry& parent, fs::path const& path) {
		if (!m_info.filter.include_files) { return; }
		auto file = Entry{.type = EntryType::File, .path = fs::canonical(path)};
		file.tags = get_combined_tags(file.path, parent.tags);
		if (!should_include(file.tags, m_info.filter)) { return; }
		parent.subentries.push_back(std::move(file));
	}

	Instance& m_self;
	ScanInfo const& m_info;
};

class FormatTree {
  public:
	explicit FormatTree(Formatter const& formatter) : m_formatter(formatter) {}

	[[nodiscard]] auto operator()(Entry const& directory) {
		if (directory.type == EntryType::Directory) { format_directory(directory, directory, 0); }
		return std::move(m_ret);
	}

	void format_directory(Entry const& parent, Entry const& directory, int const depth) {
		write_file(parent, directory, depth);
		for (auto const& subentry : directory.subentries) {
			switch (subentry.type) {
			case EntryType::Directory: format_directory(directory, subentry, depth + 1); break;
			case EntryType::File: write_file(parent, subentry, depth); break;
			default: break;
			}
		}
	}

	void prefix_spaces(int const count) {
		for (int i = 0; i < count; ++i) { m_ret.push_back(' '); }
	}

	void write_line(std::string_view const line, int const depth) {
		if (!m_ret.empty() && m_ret.back() != '\n') { m_ret.push_back('\n'); }
		prefix_spaces(depth * 2);
		m_ret.append(line);
	}

	void write_file(Entry const& parent, Entry const& file, int const depth) {
		auto line = std::string{"|-- "};
		m_formatter.format_file_to(line, file, parent.path);
		write_line(line, depth);
	}

  private:
	Formatter const& m_formatter;
	std::string m_ret{};
};
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
	auto const formatter = Formatter{.delimiter = {&tag_delimiter_v, 1}};
	for (auto const& tag : tags) { formatter.join_to(out, tag); }
}

void detail::deserialize_tags(StringSet& out_set, std::string_view const serialized, OnTagDeserialized per_tag) {
	for (auto const it : std::views::split(serialized, tag_delimiter_v)) {
		auto const tag = std::string_view{it};
		per_tag(repoint_through(out_set, tag));
	}
}

void Entry::sort_recursive() {
	auto const pred = [](Entry const& a, Entry const& b) {
		if (a.type != b.type) { return a.type == EntryType::Directory; }
		return a.path < b.path;
	};
	std::ranges::sort(subentries, pred);
	for (auto& subentry : subentries) { subentry.sort_recursive(); }
}

auto Instance::get_tags(fs::path const& path) -> Result<Entry> {
	auto& serialized = wipe_buffer();
	return get_serialized_to(serialized, path).transform([&] {
		auto ret = Entry{.path = fs::canonical(path)};
		ret.type = to_entry_type(ret.path);
		auto const per_tag = [&](std::string_view const tag) { ret.tags.push_back(ScanTag{.value = tag, .type = TagType::Primary}); };
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

auto Instance::scan_directory(fs::path const& directory, ScanInfo const& info) -> Result<Entry> { return Scanner{*this, info}(directory); }

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

void Formatter::join_to(std::string& out, std::string_view const item) const {
	if (item.empty()) { return; }
	if (!out.empty()) { out.append(delimiter); }
	out.append(item);
}

auto Formatter::join(std::string_view const item) const -> std::string {
	auto ret = std::string{};
	join_to(ret, item);
	return ret;
}

void Formatter::truncate_to(std::string& out, std::span<std::string const> items, int const max_count) const {
	for (auto const& [index, item] : std::views::enumerate(items)) {
		auto const count = index + 1;
		if (count > max_count) {
			join_to(out, truncator);
			return;
		}
		join_to(out, item);
	}
}

auto Formatter::truncate(std::span<std::string const> items, int const max_count) const -> std::string {
	auto ret = std::string{};
	truncate_to(ret, items, max_count);
	return ret;
}

auto Formatter::format(ScanTag const& tag) const -> std::string {
	if (tag.type == TagType::Primary || inherited_prefix.empty()) { return std::string{tag.value}; }
	return std::format("{}{}", inherited_prefix, tag.value);
}

void Formatter::join_to(std::string& out, std::span<ScanTag const> tags) const {
	for (auto const& tag : tags) { join_to(out, format(tag)); }
}

auto Formatter::join(std::span<ScanTag const> tags) const -> std::string {
	auto ret = std::string{};
	join_to(ret, tags);
	return ret;
}

void Formatter::format_file_to(std::string& out, Entry const& file, fs::path const& parent) const {
	auto const tags = join(file.tags);
	auto const path = [&] {
		if (parent.empty()) { return file.path.generic_string(); }
		return fs::relative(file.path, parent).generic_string();
	}();

	out += path;
	if (!tags.empty()) { std::format_to(std::back_inserter(out), " [{}]", tags); }
}

auto Formatter::format_file(Entry const& file, fs::path const& parent) const -> std::string {
	auto ret = std::string{};
	format_file_to(ret, file, parent);
	return ret;
}

auto Formatter::format_tree(Entry const& directory) const -> std::string { return FormatTree{*this}(directory); }
} // namespace xtag

auto xtag::format_error(Error::Type const type, std::string_view const message) -> std::string {
	return std::format("[{}] {}", Error::type_name_map.to_name(type), message);
}

auto xtag::to_error(Error::Type const type, std::string_view const message) -> std::unexpected<Error> {
	auto msg = format_error(type, message);
	return std::unexpected{Error{.type = type, .message = std::move(msg)}};
}
