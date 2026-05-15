#pragma once
#include "ui/controller.hpp"
#include "ui/main_menu.hpp"
#include "ui/main_window.hpp"
#include "ui/modal/modal.hpp"
#include "xtag/instance.hpp"

namespace xtag::gui::ui {
class Controller : public ui::IController, public klib::Pinned {
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

	void open_test_modal() final;

  private:
	klib::Ptr<Instance> m_instance{};

	ui::MainMenu m_main_menu{*this};
	ui::MainWindow m_main_window{*this};

	fs::path m_root{};
	State m_state{State::Running};

	Modal m_test_modal{};
};
} // namespace xtag::gui::ui
