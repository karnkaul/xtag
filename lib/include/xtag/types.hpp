#pragma once
#include "klib/enum/bitops.hpp"
#include "klib/enum/name.hpp"
#include <cstdint>
#include <string>

namespace xtag {
struct Error {
	enum class Type : std::int8_t {
		Unknown,
		InvalidArgument,
		AccessDenied,
		PathTooLong,
		NotSupported,
		NoData,
		TooBig,
	};

	inline static auto const type_name_map = klib::EnumNameMap<Type>{
		{Type::Unknown, "Unknown"},			{Type::InvalidArgument, "InvalidArgument"}, {Type::AccessDenied, "AccessDenied"},
		{Type::PathTooLong, "PathTooLong"}, {Type::NotSupported, "NotSupported"},		{Type::NoData, "NoData"},
		{Type::TooBig, "TooBig"},
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
	default: return ExitCode::Failure;
	}
}

enum class Filter : std::int8_t {
	None = 0,
	Directory = 1 << 0,
	File = 1 << 1,
};
[[nodiscard]] constexpr auto enable_enum_bitops(Filter /*unused*/) { return true; }

struct ScanParams {
	Filter filter{Filter::Directory | Filter::File};
	int depth{0};
};
} // namespace xtag
