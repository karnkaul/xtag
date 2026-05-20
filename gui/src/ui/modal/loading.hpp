#pragma once
#include "app/delta_time.hpp"
#include "klib/ptr.hpp"
#include "ui/modal/modal.hpp"

namespace xtag::gui::ui {
class LoadingModal : public Modal {
  public:
	explicit LoadingModal(DeltaTime const& delta_time) : Modal("Loading"), m_delta_time(&delta_time) {}

	std::string body_text{"Please wait"};
	int max_dots{5};
	Seconds dot_rate{250ms};

  private:
	void on_open() final;
	void on_update() final;

	void bump_dots();
	void refresh_dots();

	klib::Ptr<DeltaTime const> m_delta_time{};
	Seconds m_dot_remain{};
	std::string m_dots{};
	int m_dot_count{};
};
} // namespace xtag::gui::ui
