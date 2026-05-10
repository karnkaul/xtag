#pragma once
#include "xtag/types.hpp"
#include <expected>

namespace xtag {
template <typename Type>
using Result = std::expected<Type, Error>;

[[nodiscard]] auto format_error(Error::Type type, std::string_view message) -> std::string;
[[nodiscard]] auto to_error(Error::Type type, std::string_view message) -> std::unexpected<Error>;
} // namespace xtag
