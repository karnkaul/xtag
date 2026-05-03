#include "command/scan.hpp"
#include "xtag/format.hpp"

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
	auto scan_params = ScanParams{
		.tag_filter = m_tags,
		.depth = m_depth,
	};

	if (m_type == "d") {
		scan_params.entry_type = EntryType::Directory;
	} else if (m_type == "f") {
		scan_params.entry_type = EntryType::File;
	}

	auto const root = fs::absolute(m_path);
	auto const entries = instance.scan_tagged(root, scan_params);

	auto format_params = FormatParams{
		.path_header = "relative path",
		.transform_path = [&](fs::path const& path) { return fs::relative(path, root); },
	};

	auto const table = format_table(entries, std::move(format_params));
	std::println("{}", table);
	return ExitCode::Success;
}
} // namespace xtag::cli::command
