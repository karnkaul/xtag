#include "command/scan.hpp"

namespace xtag::cli::command {
auto Scan::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::named_option(m_depth, "d,depth", "iteration depth (default: 10)"),
		clap::named_option(m_type, "t,type", "entry type (d|f)"),
		clap::positional_required(m_path, "PATH"),
		clap::positional_list(m_tags, "TAGS"),
	};
}

auto Scan::execute(Instance& instance) -> ExitCode {
	auto params = ScanParams{
		.tag_filter = m_tags,
		.depth = m_depth,
	};

	if (m_type == "d") {
		params.entry_type = EntryType::Directory;
	} else if (m_type == "f") {
		params.entry_type = EntryType::File;
	}

	auto const result = instance.scan_tagged(m_path, params);
	auto const table = format_table(result);
	std::println("{}", table);
	return ExitCode::Success;
}
} // namespace xtag::cli::command
