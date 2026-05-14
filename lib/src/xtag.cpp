#include "detail/util.hpp"
#include "klib/cli/text_table.hpp"
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

class ScannerOld {
  public:
	explicit ScannerOld(Instance& self, ScanInfo const& info) : m_self(self), m_info(info) {}

	[[nodiscard]] auto operator()(fs::path const& root) -> Result<EntryOld> {
		if (!fs::is_directory(root)) { return to_error(Error::Type::InvalidArgument, std::format("not a directory: '{}'", root.generic_string())); }
		return scan_directory(root, {}, 0);
	}

  private:
	[[nodiscard]] auto scan_directory(fs::path const& path, std::span<ScanTag const> parent_tags, int const depth) -> EntryOld {
		auto ret = EntryOld{.type = EntryType::Directory};
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

	void scan_subdirectory(EntryOld& parent, fs::path const& path, int const depth) {
		if (depth > m_info.depth) { return; }
		auto subdir = scan_directory(path, parent.tags, depth);
		if (!should_include(subdir.tags, m_info.filter)) { return; }
		parent.subentries.push_back(std::move(subdir));
	}

	void scan_file(EntryOld& parent, fs::path const& path) {
		if (!m_info.filter.include_files) { return; }
		auto file = EntryOld{.type = EntryType::File, .path = fs::canonical(path)};
		file.tags = get_combined_tags(file.path, parent.tags);
		if (!should_include(file.tags, m_info.filter)) { return; }
		parent.subentries.push_back(std::move(file));
	}

	Instance& m_self;
	ScanInfo const& m_info;
};

class Scanner {
  public:
	explicit Scanner(Instance& self, ScanInfo const& info) : m_self(self), m_info(info) {}

	[[nodiscard]] auto operator()(fs::path const& root) -> Result<EntryList> {
		if (!fs::is_directory(root)) { return to_error(Error::Type::InvalidArgument, std::format("not a directory: '{}'", root.generic_string())); }
		m_ret.path = fs::canonical(root);
		scan_directory(m_ret.path, {}, 0);
		return std::move(m_ret);
	}

  private:
	void scan_directory(fs::path const& path, std::span<ScanTag const> parent_tags, int const depth) {
		if (depth > m_info.depth) { return; }

		auto const tags = get_combined_tags(path, parent_tags);

		if (should_include(tags, m_info.filter)) {
			m_ret.entries.push_back(Entry{
				.type = EntryType::Directory,
				.path = path,
				.tags = tags,
			});
		}

		for (auto const& it : fs::directory_iterator{path}) {
			if (it.is_directory()) {
				scan_directory(it.path(), tags, depth + 1);
			} else if (it.is_regular_file()) {
				scan_file(it.path(), tags);
			}
		}
	}

	[[nodiscard]] auto get_combined_tags(fs::path const& path, std::span<ScanTag const> inherited_tags) const -> std::vector<ScanTag> {
		auto ret = std::vector<ScanTag>{};
		if (auto result = m_self.get_tags(path)) { ret = std::move(result->tags); }
		for (auto const& scan_tag : inherited_tags) { ret.push_back(ScanTag{.value = scan_tag.value, .type = TagType::Inherited}); }
		return ret;
	}

	void scan_file(fs::path const& path, std::span<ScanTag const> inherited_tags) {
		if (!m_info.filter.include_files) { return; }
		auto file = Entry{.type = EntryType::File, .path = fs::canonical(path)};
		file.tags = get_combined_tags(file.path, inherited_tags);
		if (!should_include(file.tags, m_info.filter)) { return; }
		m_ret.entries.push_back(std::move(file));
	}

	Instance& m_self;
	ScanInfo const& m_info;

	EntryList m_ret{};
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

void EntryOld::sort_recursive() {
	auto const pred = [](EntryOld const& a, EntryOld const& b) {
		if (a.type != b.type) { return a.type == EntryType::Directory; }
		return a.path < b.path;
	};
	std::ranges::sort(subentries, pred);
	for (auto& subentry : subentries) { subentry.sort_recursive(); }
}

void EntryList::sort_entries() {
	auto const pred = [](Entry const& a, Entry const& b) {
		if (a.type != b.type) { return a.type == EntryType::Directory; }
		return a.path < b.path;
	};
	std::ranges::sort(entries, pred);
}

auto Instance::get_tags(fs::path const& path) -> Result<EntryOld> {
	auto& serialized = wipe_buffer();
	return get_serialized_to(serialized, path).transform([&] {
		auto ret = EntryOld{.path = fs::canonical(path)};
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

auto Instance::scan_directory_old(fs::path const& directory, ScanInfo const& info) -> Result<EntryOld> { return ScannerOld{*this, info}(directory); }

auto Instance::scan_directory(fs::path const& directory, ScanInfo const& info) -> Result<EntryList> { return Scanner{*this, info}(directory); }

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

void Formatter::format_to(std::string& out, Entry const& entry, fs::path const& root) const {
	auto const tags = join(entry.tags);
	auto const path = [&] {
		if (root.empty()) { return entry.path.generic_string(); }
		return fs::relative(entry.path, root).generic_string();
	}();

	out.append(path);
	if (entry.type == EntryType::Directory && !out.ends_with('/')) { out.push_back('/'); }

	if (!tags.empty()) { std::format_to(std::back_inserter(out), " [{}]", tags); }
}

auto Formatter::format(Entry const& entry, fs::path const& root) const -> std::string {
	auto ret = std::string{};
	format_to(ret, entry, root);
	return ret;
}

auto Formatter::format_table(EntryList const& list) const -> std::string {
	if (list.entries.empty()) { return {}; }

	using Align = klib::TextTable::Align;
	auto table = klib::TextTable::Builder{}.add_column("#", Align::Right).add_column("relative path").add_column("tags").build();
	for (auto const& [index, entry] : std::views::enumerate(list.entries)) {
		auto const number = index + 1;
		auto row = std::vector<std::string>{};

		row.push_back(std::format("{}", number));

		auto path = fs::relative(entry.path, list.path).generic_string();
		if (entry.type == EntryType::Directory && !path.ends_with('/')) { path.push_back('/'); }
		row.push_back(std::move(path));

		row.push_back(join(entry.tags));

		table.push_row(std::move(row));
	}
	return table.serialize();
}
} // namespace xtag

auto xtag::format_error(Error::Type const type, std::string_view const message) -> std::string {
	return std::format("[{}] {}", Error::type_name_map.to_name(type), message);
}

auto xtag::to_error(Error::Type const type, std::string_view const message) -> std::unexpected<Error> {
	auto msg = format_error(type, message);
	return std::unexpected{Error{.type = type, .message = std::move(msg)}};
}
