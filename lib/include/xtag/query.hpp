#pragma once
#include "klib/enum/bitops.hpp"
#include "xtag/types.hpp"
#include <cstdint>
#include <string_view>
#include <vector>

namespace xtag::query {
enum class Scope : std::uint8_t {
	None = 0,
	Filename = 1 << 0,
	Tag = 1 << 1,
};
constexpr auto enable_enum_bitops(Scope /*unused*/) { return true; }

struct Predicate {
	[[nodiscard]] auto is_match(std::string_view filename, std::span<ScanTag const> tags) const -> bool;

	Scope scope{Scope::Filename | Scope::Tag};
	std::string_view pattern{};
	bool invert{};
};

struct Expression {
	[[nodiscard]] auto is_match(std::string_view filename, std::span<ScanTag const> tags) const -> bool;

	std::vector<Predicate> predicates{};
};

[[nodiscard]] auto parse(std::string_view text) -> Expression;
} // namespace xtag::query
