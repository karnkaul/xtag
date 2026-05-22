#pragma once
#include "klib/enum/bitops.hpp"
#include "klib/enum/name.hpp"
#include <any>
#include <cstdint>
#include <filesystem>
#include <string>

namespace xtag {
namespace fs = std::filesystem;

struct Error {
	enum class Type : std::int8_t {
		Unknown,
		InvalidArgument,
		AccessDenied,
		PathTooLong,
		NotSupported,
		NoData,
		TooBig,
		IOError,
	};

	inline static auto const type_name_map = klib::EnumNameMap<Type>{
		{Type::Unknown, "Unknown"},
		{Type::InvalidArgument, "InvalidArgument"},
		{Type::AccessDenied, "AccessDenied"},
		{Type::PathTooLong, "PathTooLong"},
		{Type::NotSupported, "NotSupported"},
		{Type::NoData, "NoData"},
		{Type::TooBig, "TooBig"},
		{Type::IOError, "IoError"},
	};

	Type type{};
	std::string message{};
};

enum class ExitCode : std::int8_t {
	Success = EXIT_SUCCESS,
	Failure = EXIT_FAILURE,

	InvalidArgument = 101,
	AccessDenied = 102,
	PathTooLong = 103,
	NotSupported = 104,
	NoData = 105,
	TooBig = 106,
	IOError = 107,
};

[[nodiscard]] constexpr auto to_exit_code(Error::Type const type) {
	switch (type) {
	case Error::Type::Unknown: return ExitCode::Failure;
	case Error::Type::InvalidArgument: return ExitCode::InvalidArgument;
	case Error::Type::AccessDenied: return ExitCode::AccessDenied;
	case Error::Type::PathTooLong: return ExitCode::PathTooLong;
	case Error::Type::NotSupported: return ExitCode::NotSupported;
	case Error::Type::NoData: return ExitCode::NoData;
	case Error::Type::TooBig: return ExitCode::TooBig;
	case Error::Type::IOError: return ExitCode::IOError;
	default: return ExitCode::Failure;
	}
}

enum class TagType : std::int8_t {
	None = 0,
	Primary = 1 << 0,
	Inherited = 1 << 1,
	Untagged = 1 << 2,
};
[[nodiscard]] constexpr auto enable_enum_bitops(TagType /*unused*/) { return true; }
inline auto const tag_type_name_map = klib::EnumNameMap<TagType>{
	{TagType::None, "None"},
	{TagType::Primary, "Primary"},
	{TagType::Inherited, "Inherited"},
	{TagType::Untagged, "Untagged"},
};

struct ScanTag {
	using Type = TagType;

	auto operator==(ScanTag const&) const -> bool = default;

	std::string_view value{};
	Type type{};
};

enum class EntryType : std::int8_t {
	None = 0,
	Directory = 1 << 0,
	File = 1 << 1,
};
[[nodiscard]] constexpr auto enable_enum_bitops(EntryType /*unused*/) { return true; }
inline auto const entry_type_name_map = klib::EnumNameMap<EntryType>{
	{EntryType::None, "None"},
	{EntryType::Directory, "Directory"},
	{EntryType::File, "File"},
};

struct Entry {
	using Type = EntryType;

	Type type{};
	fs::path path{};
	std::vector<ScanTag> tags{};
	std::any custom_payload{};
};

struct EntryList {
	void sort_entries();

	fs::path path{};
	std::vector<Entry> entries{};
};
} // namespace xtag
