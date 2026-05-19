#pragma once
#include "ui/dispatch.hpp"
#include "ui/main_window.hpp"
#include "ui/modal/loading.hpp"
#include "xtag/instance.hpp"
#include <future>

namespace xtag::gui::ui {
class Controller : public ui::IDispatch, public klib::Pinned {
  public:
	enum class State : std::int8_t { Running, Finished };

	static void set_styles(ImGuiStyle& style);

	void initialize(Services const& services) final;
	void update() final;

	[[nodiscard]] auto get_state() const -> State { return m_state; }

	void on_drop(fs::path const& root);
	void on_window_close() { shutdown(); }

	void shutdown() final;
	void refresh_root_directory() final;
	void replace_tags(fs::path const& path, std::span<std::string_view const> tags) final;

  private:
	void poll_future();

	klib::Ptr<Instance> m_instance{};

	std::shared_ptr<EntryList> m_entry_list{};
	ui::MainWindow m_main_window{*this};

	fs::path m_root{};
	std::future<Result<EntryList>> m_future{};
	LoadingModal m_loading_modal{};
	State m_state{State::Running};
};
} // namespace xtag::gui::ui
