#pragma once
#include "app/main_menu.hpp"
#include "app/main_window.hpp"
#include "xtag/instance.hpp"
#include <cstdint>

namespace xtag::gui {
class Controller : public Object {
  public:
	enum class State : std::int8_t { Running, Finished };

	void initialize(Services const& services) final;
	void update() final;

	void on_drop(fs::path const& root);
	void on_window_close() { shutdown(); }

	[[nodiscard]] auto get_state() const -> State { return m_state; }

  private:
	void shutdown();

	std::shared_ptr<Slot> m_slot{make_slot()};

	klib::Ptr<Instance> m_instance{};

	MainMenu m_main_menu{};
	MainWindow m_main_window{};

	State m_state{State::Running};
};
} // namespace xtag::gui
