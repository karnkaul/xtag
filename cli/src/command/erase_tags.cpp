#include "command/erase_tags.hpp"

namespace xtag::cli::command {
auto EraseTags::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_optional(m_path, "PATH", "path (default=.)"),
	};
}

auto EraseTags::execute([[maybe_unused]] Instance& instance) -> ExitCode {
	auto const result = Instance::erase_tags(m_path);
	if (!result) { return handle_error(result.error()); }

	std::println("tags erased successfully");
	return ExitCode::Success;
}
} // namespace xtag::cli::command
