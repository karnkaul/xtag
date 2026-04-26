#pragma once
#include "klib/log/tagged.hpp"

namespace xtag::cli {
auto const log = klib::log::Tagged{"xtag::cli"};
} // namespace xtag::cli
