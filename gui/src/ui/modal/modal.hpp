#pragma once
#include "klib/string/c_string.hpp"
#include "ui/object.hpp"

namespace xtag::gui::ui {
class Modal : public Object {
  public:
	explicit(false) Modal(klib::CString const label = "<Untitled>") : label(label) {}

	[[nodiscard]] auto is_open() const -> bool;
	void set_should_open(bool should_open = true);
	void set_should_close(bool should_close = true);

	void update() override;

	klib::CString label{};

  protected:
	virtual void on_open() {}
	virtual void on_update();

	bool m_should_open{};
	bool m_should_close{};
};
} // namespace xtag::gui::ui
