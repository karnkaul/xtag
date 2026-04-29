#include "command/set_tags.hpp"

namespace xtag::cli::command {
auto SetTags::get_name() const -> std::string_view {
	switch (m_type) {
	default:
	case Type::Replace: return "replace-tags";
	case Type::Append: return "append-tags";
	}
}

auto SetTags::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_required(m_path, "PATH"),
		clap::positional_list(m_tags, "TAGS"),
	};
}

auto SetTags::execute(Instance& instance) -> ExitCode {
	auto const result = [&] {
		switch (m_type) {
		default:
		case Type::Replace: return instance.replace_tags(m_path, m_tags);
		case Type::Append: return instance.append_tags(m_path, m_tags);
		}
	}();
	if (!result) { return handle_error(result.error()); }

	auto const operation = [&] -> std::string_view {
		switch (m_type) {
		default:
		case Type::Replace: return "replaced";
		case Type::Append: return "appended";
		}
	}();

	std::println("tags {} successfully", operation);
	return ExitCode::Success;
}
} // namespace xtag::cli::command
