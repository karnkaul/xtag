#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace xtag::detail {
void deserialize_tags_to(std::vector<std::string>& out, std::string_view serialized);

template <typename ContainerT>
void serialize_tags_to(std::string& out, ContainerT const& tags) {
	for (auto const& tag : tags) {
		if (!out.empty()) { out.push_back('|'); }
		out.append(tag);
	}
}
} // namespace xtag::detail
