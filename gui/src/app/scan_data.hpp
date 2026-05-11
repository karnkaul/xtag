#pragma once
#include "xtag/types.hpp"

namespace xtag::gui {
struct ScanData {
	EntryType entry_type{EntryType::Directory};
	int depth{3};
};
} // namespace xtag::gui
