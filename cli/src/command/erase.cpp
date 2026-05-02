#include "command/erase.hpp"

namespace xtag::cli::command {
auto Erase::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_optional(m_path, "PATH", "path (default=.)"),
	};
}

auto Erase::execute([[maybe_unused]] Instance& instance) -> ExitCode {
	auto const result = Instance::erase_tags(m_path);
	if (!result) { return handle_error(result.error()); }

	std::println("tags erased successfully");
	return ExitCode::Success;
}
} // namespace xtag::cli::command
