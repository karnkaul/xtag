#pragma once
#include "klib/string/c_string.hpp"
#include "xtag/result.hpp"
#include "xtag/string_set.hpp"
#include <cerrno>
#include <functional>
#include <span>

namespace xtag::detail {
void serialize_tags_to(std::string& out, std::span<std::string_view const> tags);

using OnTagDeserialized = std::move_only_function<void(std::string_view)>;
void deserialize_tags(StringSet& out_set, std::string_view serialized, OnTagDeserialized per_tag);

[[nodiscard]] inline auto validate_inputs(klib::CString const path, klib::CString const name) -> Result<void> {
	if (path.as_view().empty()) { return to_error(Error::Type::InvalidArgument, "passed path is empty"); }
	if (name.as_view().empty()) { return to_error(Error::Type::InvalidArgument, "passed name is empty"); }
	return {};
}

class ScopedErrno {
  public:
	ScopedErrno(ScopedErrno const&) = delete;
	ScopedErrno(ScopedErrno&&) = delete;
	ScopedErrno& operator=(ScopedErrno const&) = delete;
	ScopedErrno& operator=(ScopedErrno&&) = delete;

	ScopedErrno() = default;
	~ScopedErrno() { errno = 0; }
};
} // namespace xtag::detail
