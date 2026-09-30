#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::set
{

/// SMT Theory Solver module for Set.
class SetSolver
{
public:
  SetSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "Set";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'Set' solver is not yet fully implemented.");
  }
};

} // namespace satie::set
