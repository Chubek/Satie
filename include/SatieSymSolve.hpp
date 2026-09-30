#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::symsolve
{

/// SMT Theory Solver module for SymSolve.
class SymSolveSolver
{
public:
  SymSolveSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "SymSolve";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'SymSolve' solver is not yet fully implemented.");
  }
};

} // namespace satie::symsolve
