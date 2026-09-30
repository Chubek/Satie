#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::fp
{

/// SMT Theory Solver module for FP.
class FPSolver
{
public:
  FPSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "FP";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'FP' solver is not yet fully implemented.");
  }
};

} // namespace satie::fp
