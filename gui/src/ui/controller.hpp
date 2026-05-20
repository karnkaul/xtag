#pragma once
#include "ui/main_window.hpp"
#include "ui/modal/loading.hpp"
#include "xtag/instance.hpp"
#include <future>

namespace xtag::gui::ui {
class Controller {
  public:
	enum class State : std::int8_t { Running, Finished };

	explicit Controller(Instance& instance, StringSet& tag_storage, DeltaTime const& delta_time);

	static void set_styles(ImGuiStyle& style);

	void update();

	[[nodiscard]] auto get_state() const -> State { return m_state; }

	void on_drop(fs::path const& root);
	void on_window_close() { shutdown(); }

  private:
	void shutdown();
	void refresh_root_directory();
	void replace_tags();

	void poll_future();

	klib::Ptr<Instance> m_instance{};

	MainWindow m_main_window;

	fs::path m_root{};
	std::future<Result<EntryList>> m_future{};
	LoadingModal m_loading_modal;
	State m_state{State::Running};
};
} // namespace xtag::gui::ui
