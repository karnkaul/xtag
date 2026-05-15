#pragma once
#include "klib/debug/assert.hpp"
#include "klib/ptr.hpp"
#include <typeindex>
#include <unordered_map>

namespace xtag::gui {
class Services {
  public:
	template <typename T>
	void attach(T* service) {
		if (!service) { return; }
		m_map.insert_or_assign(typeid(T), service);
	}

	template <typename T>
	[[nodiscard]] auto find() const -> klib::Ptr<T> {
		auto const it = m_map.find(typeid(T));
		if (it == m_map.end()) { return nullptr; }
		return static_cast<T*>(it->second);
	}

	template <typename T>
	[[nodiscard]] auto get() const -> T& {
		auto ret = find<T>();
		KLIB_ASSERT(ret);
		return *ret;
	}

  private:
	std::unordered_map<std::type_index, void*> m_map{};
};
} // namespace xtag::gui
