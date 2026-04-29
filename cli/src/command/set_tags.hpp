#pragma once
#include "command/command.hpp"

namespace xtag::cli::command {
class SetTags : public Command {
  public:
	enum class Type : std::int8_t { Replace, Append };

	explicit SetTags(Type const type) : m_type(type) {}

  private:
	[[nodiscard]] auto get_name() const -> std::string_view final;
	auto get_parameters() -> std::vector<clap::Parameter> final;
	auto execute(Instance& instance) -> ExitCode final;

	Type m_type{};
	std::string_view m_path{"."};
	std::vector<std::string_view> m_tags{};
};

class ReplaceTags : public SetTags {
  public:
	explicit ReplaceTags() : SetTags(Type::Replace) {}
};

class AppendTags : public SetTags {
  public:
	explicit AppendTags() : SetTags(Type::Append) {}
};
} // namespace xtag::cli::command
