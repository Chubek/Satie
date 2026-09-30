#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::lp
{

/// SMT Theory Solver module for LP.
class LPSolver
{
public:
  LPSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "LP";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'LP' solver is not yet fully implemented.");
  }
};

} // namespace satie::lp
