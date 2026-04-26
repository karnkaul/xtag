#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace xtag::detail {
void deserialize_tags_to(std::vector<std::string>& out, std::string_view serialized);

[[nodiscard]] inline auto deserialize_tags(std::string_view const serialized) -> std::vector<std::string> {
	auto ret = std::vector<std::string>{};
	deserialize_tags_to(ret, serialized);
	return ret;
}

template <typename ContainerT>
void serialize_tags_to(std::string& out, ContainerT const& tags) {
	for (auto const& tag : tags) {
		if (!out.empty()) { out.push_back('|'); }
		out.append(tag);
	}
}

template <typename ContainerT>
[[nodiscard]] auto serialize_tags(ContainerT const& tags) -> std::string {
	auto ret = std::string{};
	serialize_tags_to(ret, tags);
	return ret;
}

template <typename ContainerT, typename ContainerU>
void combine_tags_to(std::vector<std::string_view>& out, ContainerT const& a, ContainerU const& b) {
	out.reserve(out.size() + a.size() + b.size());
	for (std::string_view const str : a) { out.push_back(str); }
	for (std::string_view const str : b) { out.push_back(str); }
}

template <typename ContainerT, typename ContainerU>
[[nodiscard]] auto combine_tags(ContainerT const& a, ContainerU const& b) -> std::vector<std::string_view> {
	auto ret = std::vector<std::string_view>{};
	combine_tags_to(ret, a, b);
	return ret;
}
} // namespace xtag::detail
