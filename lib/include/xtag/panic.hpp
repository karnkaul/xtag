#pragma once
#include <stdexcept>

namespace xtag {
struct Panic : std::runtime_error {
	using std::runtime_error::runtime_error;
};
} // namespace xtag
