#pragma once
#include "common/test_dir.hpp"
#include "xtag/tag_storage.hpp"

namespace xtag {
struct Fixture {
	TestDir test_dir{};
	TagStorage tag_storage{};
};
} // namespace xtag
