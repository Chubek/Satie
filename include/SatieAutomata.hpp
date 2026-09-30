#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::automata
{

/// SMT Theory Solver module for Automata.
class AutomataSolver
{
public:
  AutomataSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "Automata";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'Automata' solver is not yet fully implemented.");
  }
};

} // namespace satie::automata
