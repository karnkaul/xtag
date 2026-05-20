#pragma once
#include "klib/base_types.hpp"
#include "klib/string/c_string.hpp"

namespace xtag::gui::ui {
class Modal : public klib::Polymorphic {
  public:
	explicit(false) Modal(klib::CString const label = "<Untitled>") : label(label) {}

	[[nodiscard]] auto is_open() const -> bool;
	void set_should_open(bool should_open = true);
	void set_should_close(bool should_close = true);

	void update();

	klib::CString label{};

  protected:
	virtual void on_open() {}
	virtual void on_update();

	bool m_should_open{};
	bool m_should_close{};
};
} // namespace xtag::gui::ui
