#pragma once
#include "klib/string/c_string.hpp"
#include "xtag/result.hpp"
#include <string_view>

namespace xtag::xattr {
[[nodiscard]] auto get_to(std::string& buffer, klib::CString path, klib::CString name) -> Result<std::string_view>;
[[nodiscard]] auto get(klib::CString path, klib::CString name) -> Result<std::string>;
auto set(klib::CString path, klib::CString name, klib::CString value) -> Result<void>;
auto remove(klib::CString path, klib::CString name) -> Result<void>;
} // namespace xtag::xattr
