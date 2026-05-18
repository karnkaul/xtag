#include "ui/entry_data.hpp"

namespace xtag::gui::ui {
void EntryData::write_to(Entry& out, fs::path const& root) { out.custom_payload = EntryData{.relative_path = fs::relative(out.path, root).generic_string()}; }

auto EntryData::read_from(Entry const& entry) -> EntryData const& {
	if (auto const* ret = std::any_cast<EntryData>(&entry.custom_payload)) { return *ret; }
	static auto const s_default = EntryData{};
	return s_default;
}
} // namespace xtag::gui::ui
