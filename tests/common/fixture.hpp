#pragma once
#include "common/test_dir.hpp"
#include "xtag/instance.hpp"

namespace xtag {
struct Fixture {
	TestDir test_dir{};
	Instance instance{};
};
} // namespace xtag
