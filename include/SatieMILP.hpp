#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::milp
{

/// SMT Theory Solver module for MILP.
class MILPSolver
{
public:
  MILPSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "MILP";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'MILP' solver is not yet fully implemented.");
  }
};

} // namespace satie::milp
