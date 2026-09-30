#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::bv
{

/// SMT Theory Solver module for BV.
class BVSolver
{
public:
  BVSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "BV";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'BV' solver is not yet fully implemented.");
  }
};

} // namespace satie::bv
